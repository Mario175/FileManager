#include "ItemContextMenu.hpp"
#include "FileTableView.hpp"
#include "OrderedFileSystemModel.hpp"

#include <QDebug>
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>

ItemContextMenu::ItemContextMenu(QWidget *parent) : QMenu(parent) {
	initCommon();
	this->parent = qobject_cast<FileTableView *>(parent);
    clipboard = QGuiApplication::clipboard();
	connect(this, &QMenu::aboutToHide, [&]() {
		for (const auto &action : this->actions())
			if (commonActions.contains(action))
				continue;
			else
				removeAction(action);
	});

	cutActionIndicator.reserve(4);
	cutActionIndicator[0] = 2;
	cutActionIndicator[1] = '\0';
	cutActionIndicator[2] = '\0';
	cutActionIndicator[3] = '\0';
	cutActionPadding.reserve(1044);
	cutActionPadding[0] = -1;
	cutActionPadding[1] = -1;
	cutActionPadding[2] = -1;
	cutActionPadding[3] = -1;
}

void ItemContextMenu::init() {
	selectedFiles = parent->getSelectedFiles();
    if (selectedFiles.length() == 1 && (!selectedFiles.first().fileName().compare("..") 
		|| selectedFiles.first().fileName().isEmpty())) {

		cutAction->setDisabled(true);
		copyAction->setDisabled(true);
		renameAction->setDisabled(true);
		deleteAction->setDisabled(true);
	}

	auto data = QGuiApplication::clipboard()->mimeData();
	if (!data->hasUrls())
		pasteAction->setDisabled(true);
	else
		pasteAction->setEnabled(true);

	initFolder();
	initFile();
}

void ItemContextMenu::initCommon() {
    cutAction = addAction("Cut", this, &ItemContextMenu::cutToClipboard, QKeySequence(tr("Ctrl+X")));
    copyAction = addAction("Copy", this, &ItemContextMenu::copyToClipboard, QKeySequence(tr("Ctrl+C")));
    pasteAction = addAction("Paste", this, &ItemContextMenu::pasteFromClipboard, QKeySequence(tr("Ctrl+V")));

	addSeparator();
	deleteAction = addAction("Delete selected", this, &ItemContextMenu::deleteItems);
	renameAction = addAction("Rename", this, &ItemContextMenu::rename);
	addSeparator();
    commonActions << pasteAction << copyAction << cutAction << deleteAction << renameAction;
	addSeparator();
}

void ItemContextMenu::initFile() {}
void ItemContextMenu::initFolder() {}

void ItemContextMenu::cutToClipboard() {
	auto data = parent->getModel()->mimeData(parent->getSelectedIndexes());
#ifdef WIN32
	data->setData("application/x-qt-windows-mime;value=\"Preferred DropEffect\"",cutActionIndicator);
	data->setData("application/x-qt-windows-mime;value=\"DropDescription\"",cutActionPadding);
#endif
	QGuiApplication::clipboard()->setMimeData(data);
}

void ItemContextMenu::copyToClipboard() {
	auto data = parent->getModel()->mimeData(parent->getSelectedIndexes());
	QGuiApplication::clipboard()->setMimeData(data);
}

void ItemContextMenu::pasteFromClipboard() {
	const auto &clipboard = QGuiApplication::clipboard();
	auto data = clipboard->mimeData();

	bool move = false;
#ifdef WIN32
    move = (data->data("application/x-qt-windows-mime;value=\"Preferred DropEffect\"").at(0) == 2);
#endif
	if (move) {
        parent->getModel()->dropMimeData(data, Qt::MoveAction, 1, 0, QModelIndex());
		selIndexes.clear();
		pasteAction->setDisabled(true);
	} else
		parent->getModel()->dropMimeData(data, Qt::CopyAction, 1, 0, QModelIndex());
}

void ItemContextMenu::deleteItems() {
	double start = clock();
	parent->deleteSelectedFiles();
	double end = clock();
	qDebug() << "Delete files in:" << (end-start)/double(CLOCKS_PER_SEC) << "seconds\n";
}

void ItemContextMenu::rename() {}
