#include "vsbak.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    vsbak w;
    w.show();

    return a.exec();
}
