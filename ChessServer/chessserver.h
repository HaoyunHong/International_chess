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
#include "pawnprodialog.h"

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

    int matrix[8][8];
    /*
     * 0: 无棋子
     * +: white
     * -: black
     *
     *
     * +-1: pawn
     * +-2: rook
     * +-3: horse
     * +-4: bishop
     * +-5: queen
     * +-6: king
     */

    serverConnectDialog *sCDlg;

    QTcpServer *tcpServerServer;
    QTcpSocket *tcpServerSocket;

    //菜单栏
    QMenuBar *mBar;

    //添加菜单
    QMenu *menu;
    QAction *actServer;

    QMenu *menu2;
    QAction *actInitial;
    QAction *actLoad;
    QAction *actSave;

    QString ip;
    quint16 port;

    QPoint focus;
    QVector<QPoint> focusPath;
    QVector<QPoint> curClickPath;

    void initial();

    bool isStart;

    bool isSelected;

    QPoint curLeftClick;

    QPixmap getPic(QPoint);

    void setMovePoints(QPoint);

    int step;

    QPoint opposeOrigin;
    QPoint opposeTo;

    QTimer timerStart;
    int startTime;

    QTimer timerCount;
    int countTime;

    void choice();

    void origin();
    void playAgain();

    bool isLoad;
    QFile* loadFile;
    void fileParser(QByteArray array);

    int drawLineIndex;

    bool isMine;

    pawnProDialog *pdlg;

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

};

#endif // CHESSSERVER_H
