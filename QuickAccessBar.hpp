#ifndef QUICKACCESSBAR_H
#define QUICKACCESSBAR_H

#include <QToolBar>
#include <QPushButton>
#include <QStorageInfo>

class DriveButton : public QPushButton {
	Q_OBJECT
public:
	explicit DriveButton(const QString& root, QWidget* parent = Q_NULLPTR);

public slots:
	void click();

protected:
	// virtual bool event(QEvent* e);

private:
	const QString rootPath;
};

class QuickAccessBar : public QToolBar {
	Q_OBJECT
public:
	explicit QuickAccessBar(QWidget* parent = nullptr);
	void sendSignal(const QString&) const;

signals:
	void cdTo(const QString&) const;

public slots:
	void update();

private:
    void refreshMountPoints();
	QList<QStorageInfo> volumes;
};

#endif // QUICKACCESSBAR_H
