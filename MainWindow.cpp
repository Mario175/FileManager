#include "MainWindow.hpp"
#include "ui_progressdialog.h"
#include "FileTabSelector.hpp"
#include "FileTableView.hpp"
#include "NewDirDialog.hpp"
#include "FileProgressDialog.hpp"
#include "OrderedFileSystemModel.hpp"

#include <QApplication>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    /***
     * Setup Ui
     * */

    ui->setupUi(this);
    setStatusBar(nullptr);

    ui->leftTabWidget->setLabel(ui->leftLabel);
    ui->rightTabWidget->setLabel(ui->rightLabel);

    ui->leftTabWidget->init(ui);
    ui->rightTabWidget->init(ui);

    ui->leftLabel->setText("Left");
    ui->rightLabel->setText("Right");

    movementProgress = new ProgressDialog(this);
    //worker           = new CThread();

    readSettings();

    connect(ui->quickBar,SIGNAL(cdTo(QString)),this,SLOT(cdTo(QString)));

    connect(ui->leftTabWidget,&FileTabSelector::focusAquired,[this]() { leftTabHasFocus = true;
        ui->rightTabWidget->unfocus(); });
    connect(ui->rightTabWidget,&FileTabSelector::focusAquired,[this]() { leftTabHasFocus = false;
        ui->leftTabWidget->unfocus(); });

    connect(this,SIGNAL(setFileAction(QFileInfoList,QString,FilleOperation)),
            movementProgress,SLOT(processFileAction(QFileInfoList,QString,FilleOperation)));

    connect(ui->leftTabWidget,SIGNAL(setFileAction(QFileInfoList,QString,Qt::DropAction)),this,
            SIGNAL(setFileAction(QFileInfoList,QString,Qt::DropAction)));
    connect(ui->rightTabWidget,SIGNAL(setFileAction(QFileInfoList,QString,Qt::DropAction)),this,
            SIGNAL(setFileAction(QFileInfoList,QString,Qt::DropAction)));

    setupActions();

    ui->menubar->addAction("About", this, SLOT(showAbout()));
    //connect(ui->copyWithThread, SIGNAL(clicked()), this, SLOT(on_copyWithThread_clicked()));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::showAbout() {
    QMessageBox message;
    message.setIcon(QMessageBox::Information);
    message.setWindowTitle("About");
    message.setText("Application File Manager is written in Qt version 5.15.0");
    message.setDefaultButton(QMessageBox::Ok);
    message.exec();
}

void MainWindow::setFocusSlot(FileTabSelector *tab) {
    if (leftTabHasFocus) {
        if (tab == ui->leftTabWidget)
            return;
    } else if (tab == ui->rightTabWidget)
        return;

    QEvent *event1 = new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QEvent *event2 = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Tab, Qt::NoModifier);
    qApp->postEvent(tab, event1);
    qApp->postEvent(tab, event2);
}

void MainWindow::readSettings() {
    QSettings settings;
    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(870, 580)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    bool maximized = settings.value("maximized", false).toBool();
    if (maximized)
        setWindowState(Qt::WindowMaximized);
    settings.endGroup();

    ui->action_show_hidden_files->setChecked(settings.value("showHidden", true).toBool());

    ui->rightTabWidget->readSettings();
    ui->leftTabWidget->readSettings();
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    auto key = event->key();
    auto modifier = event->modifiers();
    switch (key) {
    case Qt::Key_F5:
        copyFiles();
        break;
    case Qt::Key_F6:
        moveFiles();
        break;
    case Qt::Key_F7:
        makeDir();
        break;
    case Qt::Key_F8:
        deleteFiles();
        break;
    case Qt::Key_F9:
        renameFiles();
        break;
    default:
        QMainWindow::keyPressEvent(event);
        break;
    }
}

void MainWindow::copyFiles() {
    QFileInfoList fileList = getSelectedFiles();
    if (fileList.isEmpty())
        return;
    QString destination = getDirInFocus(true);
    if (!getDir(destination, fileList.size(), Qt::CopyAction))
        return;

    QDir dir;
    if (!dir.exists(destination)) {
        QString message = "Directory\n" + destination + "\ndoesn't exist.\nCreate it?";
        auto reply = QMessageBox::warning(this, "Warning!", message, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
        dir.mkdir(destination);
    }

    emit setFileAction(fileList, destination, Copy);
}

void MainWindow::moveFiles() {
    QFileInfoList fileList = getSelectedFiles();
    if (fileList.isEmpty())
        return;
    QString destination = getDirInFocus(true);
    if (!getDir(destination, fileList.size(), Qt::MoveAction))
        return;

    QDir dir;
    if (!dir.exists(destination)) {
        QString message = "Directory\n" + destination + "\ndoesn't exist.\nCreate it?";
        auto reply = QMessageBox::warning(this, "Warning!", message, QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
        dir.mkdir(destination);
    }

    emit setFileAction(fileList, destination, Move);
}

void MainWindow::deleteFiles() { focusedTab()->deleteSelectedFiles(); }

void MainWindow::renameFiles() {
    QMessageBox::information(this, "Info", "Rename not implemented");
};

QString MainWindow::getDirInFocus(bool opposite) {
    auto left = qobject_cast<FileTableView *>(ui->leftTabWidget->currentWidget());
    auto right = qobject_cast<FileTableView *>(ui->rightTabWidget->currentWidget());
    bool focus = leftTabHasFocus;
    if (opposite)
        focus = !leftTabHasFocus;
    if (focus)
        return left->getDirectory();
    return right->getDirectory();
}

QFileInfoList MainWindow::getSelectedFiles() {
    QFileInfoList list = focusedTab()->getSelectedFiles();
    if (1 == list.size() && 0 == list.begin()->fileName().compare(".."))
        list.clear();
    return list;
}

FileTabSelector *MainWindow::focusedSelector() {
    if (leftTabHasFocus)
        return ui->leftTabWidget;
    return ui->rightTabWidget;
}

FileTableView *MainWindow::focusedTab() {
    return qobject_cast<FileTableView *>(focusedSelector()->currentWidget());
}

void MainWindow::cdTo(const QString &dir) {
    focusedTab()->cdTo(dir);
}

bool MainWindow::getDir(QString &dirName, int numFiles, Qt::DropAction action) {
    QString message;
    switch (action) {
    case Qt::CopyAction:
        message = "Copy " + QString::number(numFiles) + " files to:";
        break;
    case Qt::MoveAction:
        message = "Move " + QString::number(numFiles) + " files to:";
        break;
    default:
        message = "New directory name:";
    }

    QLabel lbl(this);
    auto dialog = new NewDirDialog(message, dirName, &lbl);
    dialog->adjustSize();
    lbl.show();

    QRect r = geometry();
    int   x = r.x() + r.width() / 2;
    int   y = r.y() + r.height() / 2;

    dialog->move(x, y);
    int hz = dialog->exec();
    if (hz) {
        dirName = dialog->getName();
        lbl.setText(dirName);
        return true;
    }
    return false;
}

bool MainWindow::getFileName(QString &fileName) {
    QString message = "Enter file name to edit:";

    QLabel lbl(this);
    auto   dialog = new NewFileDialog(message, fileName, &lbl);
    dialog->adjustSize();
    lbl.show();

    QRect r = geometry();
    int   x = r.x() + r.width() / 2;
    int   y = r.y() + r.height() / 2;

    dialog->move(x, y);
    int hz = dialog->exec();
    if (hz) {
        fileName = dialog->getName();
        lbl.setText(fileName);
        return true;
    }
    return false;
}

void MainWindow::makeDir() {
    QDir currDir(getDirInFocus());
    QString dirName;
    if (!getDir(dirName, 0, Qt::IgnoreAction))
        return;
    bool status = currDir.mkdir(dirName);
    if (!status)
        QMessageBox::critical(this, "Error!", "Unable to create directory " + dirName + " in " + currDir.dirName());
}

void MainWindow::on_F5_clicked() {
    //double start = clock();
    copyFiles();
    //double end = clock();
    //qDebug() << "Copy files in:" << (end-start)/double(CLOCKS_PER_SEC) << "seconds\n";
}

void MainWindow::on_F6_clicked() {
    //double start = clock();
    moveFiles();
    //double end = clock();
    //qDebug() << "Move files in:" << (end-start)/double(CLOCKS_PER_SEC) << "seconds\n";
}

void MainWindow::on_F7_clicked() { makeDir(); }

void MainWindow::on_F8_clicked() {
    //double start = clock();
    deleteFiles();
    //double end = clock();
    //qDebug() << "Delete files in:" << (end-start)/double(CLOCKS_PER_SEC) << "seconds\n";

}

void MainWindow::on_F9_clicked() { renameFiles(); }

void MainWindow::focusPreviouslyFocused() {
    if (leftTabHasFocus)
        ui->leftTabWidget->currentWidget()->setFocus();
    else
        ui->rightTabWidget->currentWidget()->setFocus();
}

void MainWindow::setupActions() {
    /**
     * Files menu
     * */
    connect(ui->action_new_directory, &QAction::triggered, [&]() { makeDir(); });

    connect(ui->actionExit, &QAction::triggered,[&]() { QApplication::quit(); });
    ui->actionExit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q));

    /**
     * Actions menu
     * */
    connect(ui->F10, &QPushButton::clicked,[&]() { QApplication::quit(); });
    ui->F10->setShortcut(tr("F10"));
}

void MainWindow::on_action_show_hidden_files_changed() {
    ui->rightTabWidget->showHidden(false);
    ui->leftTabWidget->showHidden(false);
}

bool MainWindow::showHidden() {
    return ui->action_show_hidden_files->isChecked();
}

/*
void MainWindow::on_copyWithThread_clicked() {
    clock_t start = clock();
    //qDebug() << "MainThread: " << QThread::currentThread() << "\n";
    worker->start();
    clock_t end = clock();
    qDebug() << "Copy with thread TIME(SEC) " << static_cast<double>(end-start)/CLOCKS_PER_SEC;
}*/
