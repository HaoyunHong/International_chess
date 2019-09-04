#include "chessserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ChessServer w;
    w.show();

    return a.exec();
}
