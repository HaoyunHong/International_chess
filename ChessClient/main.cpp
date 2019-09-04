#include "chessclient.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChessClient w;
    w.show();

    return a.exec();
}
