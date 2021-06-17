#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ui_mainwindow.h"
//#include "CThread.hpp"

class ProgressDialog;
enum FilleOperation : int;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit            MainWindow(QWidget* parent = nullptr);
    QString             getDirInFocus(bool opposite = false);
    ~MainWindow() final;
    FileTableView*      focusedTab();
    FileTabSelector*    focusedSelector();
    ProgressDialog*     getFileMover() { return movementProgress; }
    bool			    showHidden();
//    CThread*            worker;

signals:
    void setFocus(FileTabSelector* tab);
    void setFileAction(QFileInfoList fileList, QString destination,
                       FilleOperation action);

public slots:
    void cdTo(const QString&);
    void focusPreviouslyFocused();
    void setFocusSlot(FileTabSelector* tab);

private slots:
    void on_F5_clicked();
    void on_F6_clicked();
    void on_F7_clicked();
    void on_F8_clicked();
    void on_F9_clicked();
    void on_action_show_hidden_files_changed();
//    void on_copyWithThread_clicked();
    void showAbout();

private:
    Ui::MainWindow* ui;
    ProgressDialog* movementProgress;
    bool			leftTabHasFocus;

    void copyFiles();
    void moveFiles();
    void makeDir();
    void deleteFiles();
    void renameFiles();
    void readSettings();
    void setupActions();

    QFileInfoList getSelectedFiles();
    void		  keyPressEvent(QKeyEvent *event) override;
    bool		  getDir(QString &dirName, int numFiles, Qt::DropAction action);
    bool		  getFileName(QString &fileName);

};

#endif // MAINWINDOW_H
