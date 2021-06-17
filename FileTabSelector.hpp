#ifndef FILETABSELECTOR_H
#define FILETABSELECTOR_H

#include <QTabWidget>
#include <QFileInfoList>

class FileTableView;
class QLabel;
class QMenu;

namespace Ui {
	class MainWindow;
}

class FileTabSelector : public QTabWidget {
	Q_OBJECT
public:
	FileTabSelector(QWidget* parent = Q_NULLPTR);
	~FileTabSelector() final;
	FileTableView*  addNewTab(bool dup = false, QString dir = "");
	void		    readSettings();
	void		    setLabel(QLabel* infoLabel) { this->infoLabel = infoLabel; }
	QLabel* 	    getLabel() { return infoLabel; }
	void		    init(Ui::MainWindow* ui);
	void		    showHidden(bool show);
	void		    unfocus();

public slots:
	void onDirChanged(const QString& dirName, int tabIndex);
	void onFocusEvent(bool);

signals:
	void focusAquired();

private:
	QString     defaultStyle;
	QLabel*     infoLabel{};
	QMenu*   	menu;

	QPalette defaultPalette;
	QPalette highlightedPalette;

	friend class MainWindow;
};

#endif // FILETABSELECTOR_H
