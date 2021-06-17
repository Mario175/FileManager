#include "MainWindow.hpp"

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);
	MainWindow w;
    w.setWindowTitle("FileManager");
	w.show();

	return QApplication::exec();
}
