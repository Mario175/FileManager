#ifndef FILEPROGRESSDIALOG_H
#define FILEPROGRESSDIALOG_H

#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <array>

namespace Ui {
	class ProgressDialog;
}

enum FilleOperation : int { Copy, Move, Delete };

class ProgressDialog : public QDialog {
	Q_OBJECT
public:
	explicit ProgressDialog(QWidget *parent = nullptr, 
		Qt::WindowFlags f = Qt::WindowFlags() | Qt::Window);
	~ProgressDialog() final;

signals:
	void sendErrMsg(QString errorText);
	void hideDialogSignal();
	void dirMoved(int);
	void setStatus(int status);
	void changeWindowTitle(QString);

public slots:
	void onWrite(uint);
	void movementResult(int);
	void dirMovementResult(int);
	void errorMsg(const QString &errorText);
	void hideDialogSlot() { this->hide(); }
	void processFileAction(const QFileInfoList &fileList, 
		const QString& destination, FilleOperation action);

private slots:
	void on_pauseButton_clicked();
	void on_abortButton_clicked();

private:
	Ui::ProgressDialog*    progress;
	QFuture<void>		   stub;
	bool				   status{true};
	QWaitCondition		   cond;
	QWaitCondition		   condStatus;
    std::array<QString, 2> pauseButtonLabels {"Continue", "Pause"};
	QMutex				   moverBlocker;

	void switchText();
	void processItemsInList();
	void dirParsing(QDir& dir, FilleOperation action, QString& destination, 
		QList<QString>& createdDirs);
	QMessageBox::StandardButton showError(int result);
};

#endif // FILEPROGRESSDIALOG_H
