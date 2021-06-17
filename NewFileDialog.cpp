#include "NewFileDialog.hpp"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>

NewFileDialog::NewFileDialog(QString &label, QString &dirName, QWidget *parent) : QDialog(parent) {
	auto qlabel = new QLabel(label);
	lineEdit = new QLineEdit(dirName, this);

	QPushButton *createButton = new QPushButton(tr("Ok"));
	createButton->setDefault(true);

	QPushButton *cancelButton = new QPushButton(tr("Cancel"));

	auto buttonBox = new QDialogButtonBox(Qt::Horizontal);
	buttonBox->addButton(createButton, QDialogButtonBox::AcceptRole);
	buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);

	connect(buttonBox,SIGNAL(accepted()),this,SLOT(accept()));
	connect(buttonBox,SIGNAL(rejected()),this,SLOT(reject()));

	auto lt = new QVBoxLayout;
	qlabel->adjustSize();
	lt->addWidget(qlabel);
	lt->addWidget(lineEdit);
	lt->addWidget(buttonBox);

	setLayout(lt);
}

QString NewFileDialog::getName() const { return lineEdit->text(); }
