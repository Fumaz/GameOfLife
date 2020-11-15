#include <iostream>
#include <QApplication>
#include <QDebug>
#include <QWidget>
#include <QDesktopWidget>


int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QWidget window;

    window.resize(500, 500);
    window.setWindowTitle("Conway's Game Of Life");
    window.move(QApplication::desktop()->screen()->rect().center() - window.rect().center());
    window.setFixedSize(window.size());
    window.show();

    return QApplication::exec();
}
