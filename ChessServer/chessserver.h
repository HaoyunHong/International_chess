#ifndef CHESSSERVER_H
#define CHESSSERVER_H

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

//Deal with TCP
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>

#include "serverconnectdialog.h"

namespace Ui {
class ChessServer;
}

class ChessServer : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChessServer(QWidget *parent = nullptr);
    ~ChessServer();

private:
    Ui::ChessServer *ui;

    bool matrix[8][8];

    serverConnectDialog *sCDlg;

    QTcpServer *tcpServerServer;
    QTcpSocket *tcpServerSocket;


    QString ip;
    quint16 port;




protected:
    void paintEvent(QPaintEvent *e);

};

#endif // CHESSSERVER_H
