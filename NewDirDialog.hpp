#ifndef NEWDIRDIALOG_H
#define NEWDIRDIALOG_H

#include "NewFileDialog.hpp"

class NewDirDialog : public NewFileDialog {
	Q_OBJECT
public:
    explicit NewDirDialog(QString& label, QString& dirName, QWidget* parent = nullptr);
private:
	QCompleter* fileCompleter;
};
#endif // NEWDIRDIALOG_H
