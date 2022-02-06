#include "mandelbrot.h"

#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    mandelbrot x;
    x.show();
    return QApplication::exec();
}
