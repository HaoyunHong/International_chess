#include "chessserver.h"
#include "ui_chessserver.h"

ChessServer::ChessServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChessServer)
{
    ui->setupUi(this);

    this->setFixedSize(500,700);

    this->setWindowTitle("Server");

    //菜单栏
    QMenuBar *mBar = menuBar();
    setMenuBar(mBar);

    //添加菜单
    QMenu *menu = mBar->addMenu("Option");
    QAction *actServer = menu->addAction("Listen");

    port =6666;



    //ip也是要可编辑的
    connect(actServer, &QAction::triggered,
            [=]()
    {
        tcpServerServer = nullptr;
        tcpServerSocket = nullptr;
        tcpServerServer = new QTcpServer(this);

        tcpServerServer->listen(QHostAddress::LocalHost,port);

        connect(tcpServerServer,&QTcpServer::newConnection,
                [=]()
        {
            menu->setEnabled(false);
            //actServer->setEnabled(false);
            qDebug()<<"newConnection!";
            tcpServerSocket = tcpServerServer->nextPendingConnection();
            //qDebug()<<"newConnection222!";
            //获取对方的IP和端口
            QString ip = tcpServerSocket->peerAddress().toString();
            //qDebug()<<"newConnection333!";
            quint16 port = tcpServerSocket->peerPort();
            QString temp = QString("[%1：%2]:成功连接客户端").arg(ip).arg(port);
           // ui->textEdit->setText(temp);
            qDebug()<<temp;

            connect(tcpServerSocket,&QTcpSocket::readyRead,
                    [=]()
            {
                //如果接到了客户端的信息应该怎么办
                qDebug()<<"Server readyRead!";

            });

        });

        //其实弹窗里点ok是不需要做什么操作的
        sCDlg = new serverConnectDialog(this);
        connect(sCDlg,&serverConnectDialog::canConnect,
                [=](QString ipp)
        {
            ip = ipp;
            qDebug()<<"ip = "<<ip;

        });

        //但是弹窗里点cancel就可以断开连接
        connect(sCDlg,&serverConnectDialog::cannotConnect,
                [=]()
        {
            qDebug()<<"Server Cancel!";
            if(nullptr == tcpServerSocket)
            {
                int ret = QMessageBox::question(this, "question", "Are you sure to cancel the game you initiate?", QMessageBox::Yes|QMessageBox::No);
                switch (ret)
                {
                case QMessageBox::Yes:
                    tcpServerServer->close();
                    menu->setEnabled(true);
                    break;
                case QMessageBox::No:
                    break;
                default:
                    break;
                }

                return;
            }
            //主动和客户端端口断开连接
            tcpServerServer->close();
            tcpServerSocket->disconnectFromHost();
            tcpServerSocket->close();
            tcpServerSocket = nullptr;
            qDebug()<<"Server Disconnected!";
            menu->setEnabled(true);
        });

        sCDlg->exec();

    });

    QIcon mainIcon(":/img/img/whiteQueen.png");
    this->setWindowIcon(mainIcon);

        for(int i=0;i<8;i++)
        {
            for(int j=0;j<8;j++)
            {
                matrix[i][j]=false;
            }
        }
}

ChessServer::~ChessServer()
{
    delete ui;
}

void ChessServer::paintEvent(QPaintEvent *e)
{
    QPainter p;
    p.begin(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    QBrush brush;
    brush.setColor(QColor(250,234,211));
    brush.setStyle(Qt::SolidPattern);

    QPixmap wood(":/img/img/wood.jpg");
    p.drawPixmap(0,0,width(),height(),wood);

    QPixmap frame(":/img/img/kuang.jpg");
    p.drawPixmap(40,70,420,420,frame);

    QRect rec = QRect(50,80,400,400);
    QPixmap back(":/img/img/draw.jpg");
    p.drawPixmap(rec,back);

    int unit = 50;

    p.translate(50,80);

    QPen pen;
    pen.setColor(QColor(106,64,40));

    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    for(int i=1;i<8;i++)
    {
        p.drawLine(i*unit,0,i*unit,400);
        p.drawLine(0,i*unit,400,i*unit);
    }

    brush.setColor(QColor(106,64,40));
    //brush.setStyle(Qt::SolidPattern);
    p.setBrush(brush);

    QPixmap backb(":/img/img/backb.jpg");
    for(int i=0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            if((i+j)%2==1)
            {
                p.drawPixmap(i*unit,j*unit,unit,unit,backb);
            }
        }
    }

    p.end();
}



