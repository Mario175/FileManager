#include "OrderedFileSystemModel.hpp"
#include "FileProgressDialog.hpp"

#include <QFileSystemModel>

OrderedFileSystemModel::OrderedFileSystemModel(QObject *parent) : QSortFilterProxyModel(parent) {}

bool OrderedFileSystemModel::lessThan(const QModelIndex &left, const QModelIndex &right) const {
  const QFileInfo &leftFile = sourceModel()->fileInfo(left);
  const QFileInfo &rightFile = sourceModel()->fileInfo(right);

  if (leftFile.isDir() ^ rightFile.isDir()) {
	if (this->order)
	  return !leftFile.isDir();
    return leftFile.isDir();
  }

  bool res = false;
  if (!leftFile.fileName().compare(".."))
	return order == Qt::AscendingOrder;
  if (!rightFile.fileName().compare(".."))
	return order != Qt::AscendingOrder;

  switch (column) {
  case 0:
	res = leftFile.fileName().compare(rightFile.fileName()) < 0;
	break;
  case 1:
	  res = leftFile.size() < rightFile.size();
	  break;
  case 2:
	if (leftFile.isDir())
	  res = leftFile.fileName().compare(rightFile.fileName()) < 0;
	else
	  res = leftFile.suffix().compare(rightFile.suffix()) < 0;
	break;
  case 3:
	res = leftFile.created().toTime_t() < rightFile.created().toTime_t();
	break;
  }
  return res;
}

void OrderedFileSystemModel::sort(int column, Qt::SortOrder order) {
  this->column = column;
  this->order = order;
  QSortFilterProxyModel::sort(column, order);
  emit directoryLoaded("");
}

QFileSystemModel *OrderedFileSystemModel::sourceModel() const {
  return dynamic_cast<QFileSystemModel *>(QSortFilterProxyModel::sourceModel());
}

bool OrderedFileSystemModel::dropMimeData(const QMimeData *data,
										  Qt::DropAction action, int row, int,
										  const QModelIndex &) {
  auto uriList = data->urls();
  QFileInfoList itemsToMove;

  auto dir = rootPath();
  for (const auto &uri : uriList)
	itemsToMove << uri.toLocalFile();
  if (0 == row) // selection is ".."
	dir.truncate(dir.lastIndexOf('/'));
  emit setFileAction(itemsToMove, dir, action);
  return true;
}

Qt::ItemFlags OrderedFileSystemModel::flags(const QModelIndex &index) const {
  Qt::ItemFlags defaultFlags = QSortFilterProxyModel::flags(index);
  if (0 == index.column())
	defaultFlags |= Qt::ItemIsEditable;
  if (index.isValid())
	  return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;

  return Qt::ItemIsDropEnabled | defaultFlags;
}

QString OrderedFileSystemModel::rootPath() {
  return sourceModel()->rootDirectory().absolutePath();
}

QModelIndex OrderedFileSystemModel::setRootPath(const QString &rootPath) const {
  return mapFromSource(sourceModel()->setRootPath(rootPath));
}

QModelIndex OrderedFileSystemModel::getRootIndex() const {
  return mapFromSource(sourceModel()->index(sourceModel()->rootPath()));
}

QModelIndex OrderedFileSystemModel::getSourceRootIndex() const {
  return (sourceModel()->index(sourceModel()->rootPath()));
}

QFileInfo OrderedFileSystemModel::fileInfo(QModelIndex index) const {
  return sourceModel()->fileInfo(mapToSource(index));
}

void OrderedFileSystemModel::setFilter(QDir::Filters filt) const {
  sourceModel()->setFilter(filt);
}
