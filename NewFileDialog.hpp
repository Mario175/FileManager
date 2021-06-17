#ifndef NEWFILEDIALOG_HPP
#define NEWFILEDIALOG_HPP

#include <QDialog>

class QLineEdit;
class QCompleter;

class NewFileDialog : public QDialog {
	Q_OBJECT
public:
    explicit NewFileDialog(QString& label, QString& dirName, QWidget* parent = nullptr);
	[[nodiscard]] QString getName() const;

protected:
	[[nodiscard]] QLineEdit* getLineEdit() const { return lineEdit; }

private:
	QLineEdit* lineEdit;
};

#endif // NEWFILEDIALOG_HPP
