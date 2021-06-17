#include "FileMoverDelegate.hpp"
#include "FileProgressDialog.hpp"

#include <QDebug>
#include <QStorageInfo>
#include <utility>
#include <array>

/*
#ifdef _WIN32
#include <windows.h>
#endif
*/

static constexpr auto MAX_READ = 1048576;
static constexpr auto MOVE_OK = 10;
static constexpr auto COPY_OK = 1;

FileMoverDelegate::FileMoverDelegate(QString _source, QString _destination, FilleOperation _action, QObject* parent)
	: QObject(parent), source(std::move(_source)), destination(std::move(_destination)), action{ _action }, status(true) {
	// qDebug()<<"Mover constructor"<<thread();
}

FileMoverDelegate::~FileMoverDelegate() {
	int res = 0;
	if (Copy == action) {
		res = this->copy();
	}
	else {
		if (isMovable(source, destination))
			res = this->move();
		else
			res = this->copy();
		if (MOVE_OK == res || COPY_OK == res)
			QFile::remove(source);
	}
	emit completed(res);
}

bool isMovable(QString &from, QString &to) {
	QStorageInfo in;
	QStorageInfo out;
	in.setPath(from);
	out.setPath(to.mid(0, to.lastIndexOf('/')));
	return in == out;
}

int FileMoverDelegate::copy() {
	QFile sourceFile(source);
	QFile destinationFile(destination);

	sourceFile.open(QIODevice::ReadOnly);
    destinationFile.open(QIODevice::WriteOnly | QIODevice::Truncate);

	qint64 totalSize = sourceFile.size();
	qint64 tempSize = 0;

    std::array<char, MAX_READ> buffer {}; // 1 Mb
	qint64					   bytesRead = 0;
	QMutex					   blocker;
	blocker.lock();

	bytesRead = sourceFile.read(buffer.data(), MAX_READ);
	while (bytesRead > 0) {
		switch (status) {
		case 0:
			cond.wait(&blocker);
			break;
		case -1:
			sourceFile.close();
			destinationFile.close();
			destinationFile.remove();
			return true;
		}

		if (destinationFile.write(buffer.data(), bytesRead) < 0)
			return false;
		tempSize += bytesRead;
		emit bytesProgress(static_cast<uint>(tempSize * 100. / totalSize * 1.));
		bytesRead = sourceFile.read(buffer.data(), MAX_READ);
	}

	blocker.unlock();
	sourceFile.close();

	destinationFile.setPermissions(sourceFile.permissions());
	destinationFile.close();

	if (bytesRead < 0) {
		// qDebug()<<sourceFile.errorString();
		return false;
	}
	return true;
}

int FileMoverDelegate::move() {
	return MOVE_OK + static_cast<int>(QFile::rename(source, destination));
}

/*
void FileMover::execute() {}
*/

void FileMoverDelegate::setStatus(int _status) {
	this->status = _status;
	// qDebug()<<"status "<<status<<" emmitted";
	cond.wakeOne();
}
