#ifndef CHESSCLIENT_H
#define CHESSCLIENT_H

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
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>

#include "clientconnectdialog.h"

namespace Ui {
class ChessClient;
}

class ChessClient : public QMainWindow
{
    Q_OBJECT

public:
    explicit ChessClient(QWidget *parent = nullptr);
    ~ChessClient();

private:
    Ui::ChessClient *ui;

    int matrix[8][8];

    clientConnectDialog *cCDlg;

    QTcpSocket *tcpClientSocket;

    QString ip;
    quint16 port;

    QPoint focus;
    QVector<QPoint> focusPath;
    QVector<QPoint> curClickPath;

    void initial();

    bool isSelected;

    bool hasDestination;

    QPoint curLeftClick;

    QPixmap getPic(QPoint);

    void setMovePoints(QPoint);

    int step;

    QPoint opposeOrigin;
    QPoint opposeTo;


protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
};

#endif // CHESSCLIENT_H
