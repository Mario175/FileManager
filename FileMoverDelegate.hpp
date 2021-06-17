#ifndef FILEMOVERDELEGATE_H
#define FILEMOVERDELEGATE_H

#include <QObject>
#include <QFile>
#include <QtConcurrent/QtConcurrent>

bool isMovable(QString &from, QString &to);
enum FilleOperation : int;

class FileMoverDelegate : public QObject {
	Q_OBJECT
public:
	explicit FileMoverDelegate(QString source, QString destination, 
		FilleOperation action, QObject* parent = nullptr);
	~FileMoverDelegate() override;

signals:
	void bytesProgress(uint);
	void completed(int);

public slots:
	void setStatus(int status);

private:
	QString		   source;
	QString		   destination;
	FilleOperation action;
	int			   status;
	QWaitCondition cond;

	int copy();
	int move();
};

#endif // FILEMOVERDELEGATE_H
