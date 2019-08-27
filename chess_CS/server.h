#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>

#include <QCloseEvent>

//Deal with picture
#include<QPainter>
#include<QPaintEvent>

//Deal with pop dialogue
#include<QAction>
#include<QDialog>
#include<QMessageBox>

//Deal with file
#include<QFile>
#include<QFileDialog>
#include<QDebug>

//Deal with Time
#include<QTime>
#include<QTimer>

//Deal with Calculation
#include<QtMath>
#include<QVector>
#include<QList>

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>

namespace Ui {
class Server;
}

class Server : public QMainWindow
{
    Q_OBJECT

public:
    explicit Server(QWidget *parent = nullptr);
    ~Server();

    void initServer();

private:
    Ui::Server *ui;

    QTcpServer  *listenSocket;
    QTcpSocket  *readWriteSocket;

    bool matrix[8][8];

protected:
    void paintEvent(QPaintEvent *e);
};

#endif // SERVER_H
