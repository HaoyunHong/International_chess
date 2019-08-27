#include "server.h"
#include "client.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Server s;
    s.show();
    Client c;
    c.show();

    return a.exec();
}
