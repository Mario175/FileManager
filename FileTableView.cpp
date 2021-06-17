#include "FileTableView.hpp"
#include "MainWindow.hpp"
#include "FileTabSelector.hpp"
#include "ItemContextMenu.hpp"
#include "OrderedFileSystemModel.hpp"

#include <QMessageBox>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QInputDialog>
#include <QKeyEvent>

static constexpr auto IN = 1;
static constexpr auto OUT = 0;

FileTableView::FileTableView(const QDir &directory, QWidget *parent)
	: QTableView(parent), directory(directory.absolutePath()), slowDoubleClickTimer(this) {
    connect(&slowDoubleClickTimer, &QTimer::timeout, this, [&] { slowDoubleClick = false; });
	this->parent = (qobject_cast<FileTabSelector *>(parent));
	infoLabel = this->parent->getLabel();
	menu = new ItemContextMenu(this);
	model = new OrderedFileSystemModel(this);
	prevRow = -1;
}

void FileTableView::on_doubleClicked(const QModelIndex &index) {
    QFileInfo info = model->fileInfo(index);
	if (info.isDir()) {
		if (info.fileName().compare(".."))
			chDir(index, IN);
		else
			chDir(index, OUT);
    }
}

void FileTableView::chDir(const QModelIndex &index, bool in_out) {
	prevRow = -1;
	if (in_out == IN) {
        directory = "..";
		QDir parentDir(model->fileInfo(index).absoluteFilePath());
		auto rootIndex = model->setRootPath(parentDir.absolutePath());
		assert(rootIndex.isValid());
		parentDir.cd(".");
		setRootIndex(model->getRootIndex());
	} else {
		QDir parentDir(model->rootPath());
		if (parentDir.isRoot())
			return;
		directory = parentDir.dirName();
		parentDir.cdUp();
		setRootIndex(model->setRootPath(parentDir.absolutePath()));
	}
	selectionModel()->clear();
	updateInfo();
	emit dirChanged(model->rootPath(), this->index);
}

void FileTableView::init() {
    setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
	setSelectionBehavior(QAbstractItemView::SelectRows);
	setTabKeyNavigation(false);

	horizontalHeader()->setStretchLastSection(true);
	horizontalHeader()->setSectionsMovable(true);
	horizontalHeader()->setSectionsClickable(true);
	horizontalHeader()->setSortIndicatorShown(true);

	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader()->setDefaultSectionSize(fontMetrics().height() + 4);

	auto fModel = new QFileSystemModel(this);
	model->setSourceModel(fModel);
	auto modelRootIndex = model->setRootPath(this->directory);
	assert(modelRootIndex.isValid());
	auto topWidgets = QApplication::topLevelWidgets();

	auto filters = QDir::AllEntries | QDir::NoDot | QDir::System;
	for (auto widget : topWidgets) {
		auto topWidget = qobject_cast<MainWindow *>(widget);
		if (topWidget) {
			if (topWidget->showHidden())
				filters |= QDir::Hidden;
			break;
		}
	}

	model->setFilter(filters);
	setSelectionMode(QAbstractItemView::NoSelection);
	setModel(model);
	model->setFilterRegExp("");
	setRootIndex(modelRootIndex);
	verticalHeader()->setVisible(false);

    connect(this,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(on_doubleClicked(QModelIndex)));
    connect(horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(headerClicked(int)));
    selectionModel()->select(model->index(1, 0, rootIndex()), QItemSelectionModel::Current);

	connect(fModel, &QFileSystemModel::directoryLoaded, [&]() {
		prevRow = model->mapToSource(currentIndex()).row();
		model->sort();
	});

    connect(model,SIGNAL(directoryLoaded(QString)),this,SLOT(setCurrentSelection(QString)));
    connect(selectionModel(),&QItemSelectionModel::selectionChanged,this,&FileTableView::updateInfo);
	connect(selectionModel(),&QItemSelectionModel::currentChanged,this,
			[this](QModelIndex current, QModelIndex prev) {
				for (int i = 0; i < 4; i++)
					update(current.sibling(current.row(), i));
				for (int i = 0; i < 4; i++)
					update(prev.sibling(prev.row(), i));
			});

    connect(model,SIGNAL(setFileAction(QFileInfoList,QString,Qt::DropAction)),
		     this,SIGNAL(setFileAction(QFileInfoList,QString,Qt::DropAction)));
	setDragEnabled(true);
	setDragDropMode(QAbstractItemView::DragDrop);
	setDropIndicatorShown(true);
    connect(this,SIGNAL(contextMenuRequested(QPoint)),this,SLOT(openContextMenu(QPoint)));
}

void FileTableView::setCurrentSelection(const QString &) {
	if (prevRow > 0) {
		selectionModel()->currentChanged(currentIndex(), currentIndex());
		return;
	}
	int  rows = model->rowCount(rootIndex());
	auto rootIndex = model->getRootIndex();
	auto index = model->index(0, 0, rootIndex);
	int  i;
	for (i = 0; i < rows; i++) {
		auto ind = model->index(i, 0, rootIndex);
		if (!directory.compare(model->fileInfo(ind).fileName())) {
			index = ind;
			break;
		}
	}
	selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
	prevRow = model->mapToSource(index).row();
	scrollTo(index);
}

void FileTableView::headerClicked(int section) {
	prevRow = model->mapToSource(currentIndex()).row();
	Qt::SortOrder order = Qt::AscendingOrder;
	if (section == horizontalHeader()->sortIndicatorSection())
		if (Qt::AscendingOrder == horizontalHeader()->sortIndicatorOrder())
			order = Qt::DescendingOrder;
	model->sort(section, order);
}

void FileTableView::focusInEvent(QFocusEvent *event) {
	QAbstractItemView::focusInEvent(event);
	emit focusEvent(true);
	updateInfo();
}

void FileTableView::focusOutEvent(QFocusEvent *event) {
	QAbstractItemView::focusOutEvent(event);
	emit focusEvent(false);
}

QFileInfoList FileTableView::getSelectedFiles() {
	QFileInfoList   selectedFiles;
	QModelIndexList items = getSelectedIndexes();
	for (const auto &fileIndex : items)
		selectedFiles.append(model->fileInfo(fileIndex));
	return selectedFiles;
}

QModelIndexList FileTableView::getSelectedIndexes() {
	QModelIndexList items = selectionModel()->selectedRows();
    auto currIdx = model->fileInfo(selectionModel()->currentIndex());
	if (items.empty() && currIdx.fileName().compare(".."))
		items.append(selectionModel()->currentIndex());
	return items;
}

void FileTableView::cdTo(const QString &dir) {
	auto rootIndex = model->setRootPath(dir);
	assert(rootIndex.isValid());
	setRootIndex(rootIndex);
	emit dirChanged(dir, this->index);
	updateInfo();
}

void FileTableView::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::RightButton) {
        auto index = indexAt(event->pos());
        if (index.isValid())
            setCurrentIndex(index);
        emit contextMenuRequested(event->pos());
        return;
    }

    slowDoubleClickTimer.start(1000);
    slowDoubleClick = true;

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    QTableView::mousePressEvent(event);
    setSelectionMode(QAbstractItemView::NoSelection);
}

void FileTableView::mouseReleaseEvent(QMouseEvent *event) {
    auto modifiers = qApp->keyboardModifiers();
    if (!(modifiers & (Qt::ControlModifier | Qt::ShiftModifier)))
        clearSelection();
    QTableView::mouseReleaseEvent(event);
}

void FileTableView::updateInfo() {
	if (!isCurrent())
		return;
	QStorageInfo storage(model->rootPath());
	QString fmt;
    auto sizeTotal = QLocale::system().formattedDataSize(storage.bytesTotal(), 2, QLocale::DataSizeTraditionalFormat);
	auto sizeRemaining = QLocale::system().formattedDataSize(storage.bytesAvailable(), 2, QLocale::DataSizeTraditionalFormat);

	fmt += model->rootPath();
	fmt += "\n";
    fmt += QString::number(selectionModel()->selectedRows().size())
                + " selected of " + QString::number(model->rowCount(rootIndex()) - 1) + " directory items";
	infoLabel->setText(fmt);
}

void FileTableView::goToFile(const QString &fullFilePath) {
	QFileInfo info(fullFilePath);
	cdTo(info.absolutePath());
	directory = info.fileName();
	prevRow = -1;
	setCurrentSelection("");
}

void FileTableView::openContextMenu(QPoint) {
	menu->init();
	menu->popup(QCursor::pos());
}

void FileTableView::deleteSelectedFiles() {
	QFileInfoList fileList = getSelectedFiles();
    QString message = "Delete " + QString::number(fileList.size()) + " files ?\n";

	QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Deleting ", message, QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::No)
		return;

	auto index = selectionModel()->selectedRows().isEmpty() 
		? selectionModel()->currentIndex() 
		: selectionModel()->selectedRows().first();
	prevRow = model->mapToSource(index).row();

	bool status;
	int  counter = 0;
    bool noToAll = false;
    bool yesToAll = false;
	foreach (auto fileInfo, fileList) {
		if (!fileInfo.fileName().compare("..", Qt::CaseInsensitive))
			continue;
		if (!fileInfo.fileName().compare(".", Qt::CaseInsensitive))
			continue;
		if (!fileInfo.isWritable()) {
            QMessageBox::warning(this, "Error!", "You don't have permissions to delete\n" + fileInfo.fileName());
			return;
		}
		if (fileInfo.isDir()) {
			QDir dir(fileInfo.absoluteFilePath());
            if (dir.count() && !yesToAll) {
                if (noToAll)
                    continue;
				QMessageBox::StandardButton reply;
                QString	message = "Are you sure you want to delete it ?\n";
                reply = QMessageBox::question(this, "Deleting Directory", message, QMessageBox::Yes | QMessageBox::No);
				if (reply == QMessageBox::No)
					break;
			}
			status = dir.removeRecursively();
		} else
			status = QFile::remove(fileInfo.absoluteFilePath());
		if (!status) {
			QString msg = "Unable to delede ";
			msg.append(fileInfo.filePath());
			QMessageBox::warning(this, "Error!", msg);
		}
		counter++;
	}
}

void FileTableView::showHidden(bool show) {
	auto filters = QDir::AllEntries | QDir::NoDot | QDir::System;
    if (show) filters |= QDir::Hidden;
	model->setFilter(filters);
}

QString FileTableView::getDirectory() { return model->rootPath(); }

bool FileTableView::isCurrent() const {
	return parent->currentWidget() == this;
}
