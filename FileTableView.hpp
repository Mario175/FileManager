#ifndef FILETABLEVIEW_H
#define FILETABLEVIEW_H

#include <QDir>
#include <QTableView>
#include <QTimer>

class OrderedFileSystemModel;
class QLabel;
class ItemContextMenu;
class FileTabSelector;

class FileTableView : public QTableView {
	Q_OBJECT
public:
	explicit FileTableView(const QDir& directory, QWidget* parent = nullptr);
	FileTableView(QWidget* parent) : FileTableView(QDir::homePath(), parent) {}

	void init();
	QFileInfoList					 getSelectedFiles();
	void							 cdTo(const QString&);
	OrderedFileSystemModel* 		 getModel() { return model; }
	QModelIndexList					 getSelectedIndexes();
	QString							 getDirectory();
	void setTabOrderIndex(int index) { this->index = index; }
	void setLabel(QLabel* infoLabel) { this->infoLabel = infoLabel; }

	void								  goToFile(const QString &fullFilePath);
	void								  deleteSelectedFiles();
	void								  showHidden(bool show);
	[[nodiscard]] OrderedFileSystemModel* getModel() const { return model; }

signals:
	void dirChanged(QString dirName, int index);
	void focusEvent(bool);
	void setFileAction(QFileInfoList, QString, Qt::DropAction);
	void contextMenuRequested(QPoint);

public slots:
	void on_doubleClicked(const QModelIndex& index);
	void setCurrentSelection(const QString&);
	void headerClicked(int section);
	void updateInfo();
	void openContextMenu(QPoint);

protected:
	void			   chDir(const QModelIndex& index, bool in_out);
	void			   focusInEvent(QFocusEvent* event) override;
	void			   focusOutEvent(QFocusEvent* event) override;
	[[nodiscard]] bool isCurrent() const;

private:
    int						index{};
	int						prevRow = -1;
	bool					slowDoubleClick = false;
	QLabel* 				infoLabel = nullptr;
	ItemContextMenu*		menu;
	QString					directory;
	QTimer					slowDoubleClickTimer;
    OrderedFileSystemModel* model{};
	FileTabSelector*		parent = nullptr;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
};

#endif // FILETABLEVIEW_H
