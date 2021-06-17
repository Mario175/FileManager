#include "FileTabSelector.hpp"
#include "MainWindow.hpp"
#include "FileTableView.hpp"

#include <QHeaderView>
#include <QSettings>
#include <QDebug>
#include <QMouseEvent>

FileTabSelector::FileTabSelector(QWidget *parent) : QTabWidget(parent) {
	defaultStyle = styleSheet();
	setStyle(parent->style());
	defaultPalette = highlightedPalette = parent->style()->standardPalette();
	menu = new QMenu(this);
}

QString showableName(const QString& dirName) {
	QString newName = dirName;
	if (dirName.length() > 20) {
		QStringList dirs = dirName.split('/', QString::SkipEmptyParts);
		if (dirs.size() < 2)
			return newName;
		if (!dirs.first().compare("/"))
			dirs.pop_front();
		newName = "../" + dirs.last();
	}
	return newName;
}

void FileTabSelector::init(Ui::MainWindow *ui) {
	setTabsClosable(false);
	tabBar()->setFocusPolicy(Qt::NoFocus);
	for (const auto &child : tabBar()->children()) {
		if (child->isWidgetType())
			qobject_cast<QWidget *>(child)->setFocusPolicy(Qt::NoFocus);
	}

	defaultPalette.setColor(QPalette::Window,highlightedPalette.color(QPalette::Mid));
	highlightedPalette.setColor(QPalette::Window,highlightedPalette.color(QPalette::Highlight));
	highlightedPalette.setColor(QPalette::WindowText,highlightedPalette.color(QPalette::HighlightedText));
}

void FileTabSelector::onDirChanged(const QString& dirName, int tabIndex) {
	setTabText(tabIndex, showableName(dirName));
	setTabToolTip(tabIndex, dirName);
	tabBar()->setDrawBase(true);
	tabBar()->setExpanding(true);
	tabBar()->setMovable(true);
}

void FileTabSelector::onFocusEvent(bool focused) {
	if (focused) {
		infoLabel->setPalette(highlightedPalette);
		emit focusAquired();
	}
}

void FileTabSelector::unfocus() {
	infoLabel->setPalette(defaultPalette);
#if QT_VERSION_MAJOR == 5 && QT_VERSION_MINOR < 9
	setStyleSheet(defaultStyle);
	setStyleSheet("selection-background-color: lightblue");
	setStyleSheet(defaultStyle);
#endif
}

FileTableView *FileTabSelector::addNewTab(bool dup, QString dir) {
    FileTableView *newTab;
	int index=0;
	if (dup) {
		index = currentIndex();
		dir = qobject_cast<FileTableView *>(widget(index))->getDirectory();
	}
	newTab = new FileTableView(dir, this);
	newTab->init();

	index = addTab(newTab, newTab->getDirectory());
	setTabText(index, showableName(newTab->getDirectory()));
	setTabToolTip(index, newTab->getDirectory());
	newTab->setTabOrderIndex(index);
    connect(newTab,SIGNAL(dirChanged(QString,int)),this,SLOT(onDirChanged(QString,int)));
	connect(newTab,SIGNAL(focusEvent(bool)),this,SLOT(onFocusEvent(bool)));

	newTab->setLabel(infoLabel);
	infoLabel->setAutoFillBackground(true);
	auto defaultState = newTab->horizontalHeader()->saveState(); // header state

	QSettings settings;
	QString settingsSection("LeftColumns");
	if (this->objectName().startsWith("right"))
		settingsSection = {"RightColumns"};

    auto headerState = settings.value(settingsSection, defaultState).toByteArray();
	auto header = newTab->horizontalHeader();
	header->restoreState(headerState);

	setCurrentIndex(index);

	newTab->headerClicked(header->sortIndicatorSection());
	newTab->headerClicked(header->sortIndicatorSection());
	newTab->setFocus();

	emit currentChanged(index);
	return newTab;
}

FileTabSelector::~FileTabSelector() {
	QSettings settings;
	int count = this->count();
	settings.beginWriteArray(objectName(), count);
	for (int i = 0; i < count; i++) {
		settings.setArrayIndex(i);
		settings.setValue("dir", qobject_cast<FileTableView *>(widget(i))->getDirectory());
	}
	settings.endArray();
}

void FileTabSelector::readSettings() {
	QSettings settings;
	int count = settings.beginReadArray(objectName());
	int i = 0;
	do {
		settings.setArrayIndex(i++);
		addNewTab(false, settings.value("dir").toString());
	} while (--count > 0);
	settings.endArray();
}

void FileTabSelector::showHidden(bool show) {
	int count = this->count();
	for (int i = 0; i < count; i++)
		qobject_cast<FileTableView *>(widget(i))->showHidden(show);
}
