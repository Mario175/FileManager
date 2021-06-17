#include "NewDirDialog.hpp"

#include <QLineEdit>
#include <QCompleter>
#include <QDirModel>

NewDirDialog::NewDirDialog(QString &label, QString &dirName, QWidget *parent)
    : NewFileDialog(label, dirName, parent) {

	fileCompleter = new QCompleter(this);
	auto dm = new QDirModel(fileCompleter);
	fileCompleter->setModel(dm);
	fileCompleter->setCompletionMode(QCompleter::PopupCompletion);
	if (!dirName.isEmpty())
		getLineEdit()->setCompleter(fileCompleter);
}
