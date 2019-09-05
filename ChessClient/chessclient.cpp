#include "chessclient.h"
#include "ui_chessclient.h"

ChessClient::ChessClient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChessClient)
{
    ui->setupUi(this);
    this->setFixedSize(500,700);

    this->setWindowTitle("Client");

    //菜单栏
    QMenuBar *mBar = menuBar();
    setMenuBar(mBar);

    //添加菜单
    QMenu *menu = mBar->addMenu("Option");
    QAction *actClient = menu->addAction("Connect");

    QMenu *menu2 = mBar->addMenu("Game");
    QAction *actLoad = menu2->addAction("load game");
    QAction *actSave = menu2->addAction("save game");

    port =6666;

    isInitial = false;

    menu2->setEnabled(false);



    connect(actClient, &QAction::triggered,
            [=]()
    {
        tcpClientSocket = new QTcpSocket(this);

        clientConnectDialog *cCDlg = new clientConnectDialog(this);


       connect(cCDlg,&clientConnectDialog::canConnect,
                [=](QString ipp)
        {
            ip = ipp;

            qDebug()<<"Client: ip = "<<ip;


            tcpClientSocket->connectToHost(QHostAddress(ip),port);

            connect(tcpClientSocket,&QTcpSocket::connected,
                    [=]()
            {
                this->setWindowTitle("Client is connected to server");
                qDebug()<<"客户端已连接";
                menu->setEnabled(false);
                menu2->setEnabled(true);

                connect(tcpClientSocket,&QTcpSocket::readyRead,
                        [=]()
                {
                    QString str = tcpClientSocket->readAll();
                    qDebug()<<"tcpClientSocket->readAll(): "<<str;

                    if(str=="0")
                    {
                        isInitial = true;
                        initial();

                        actLoad->setEnabled(false);
                        update();
                    }

                });

            });

        });
       connect(tcpClientSocket,&QTcpSocket::disconnected,
               [=]()
       {
           this->setWindowTitle("Server canceled the connection");
           qDebug()<<"客户端已断开";

           if(nullptr == tcpClientSocket)
           {
               return;
           }
           menu->setEnabled(true);
           tcpClientSocket->close();
           tcpClientSocket = nullptr;

       });

        cCDlg->exec();

    });



    QIcon mainIcon(":/img/img/blackQueen.png");
    this->setWindowIcon(mainIcon);

        for(int i=0;i<8;i++)
        {
            for(int j=0;j<8;j++)
            {
                matrix[i][j]=0;
            }
        }


        focus = QPoint(-1,-1);
}

ChessClient::~ChessClient()
{
    delete ui;
}

void ChessClient::paintEvent(QPaintEvent *e)
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

    QPixmap wPawn(":/img/img/whitePawn.png");
    QPixmap bPawn(":/img/img/blackPawn.png");
    QPixmap wRook(":/img/img/whiteRook.png");
    QPixmap bRook(":/img/img/blackRook.png");
    QPixmap wHorse(":/img/img/whiteHorse.png");
    QPixmap bHorse(":/img/img/blackHorse.png");
    QPixmap wBishop(":/img/img/whiteBishop.png");
    QPixmap bBishop(":/img/img/blackBishop.png");
    QPixmap wQueen(":/img/img/whiteQueen.png");
    QPixmap bQueen(":/img/img/blackQueen.png");
    QPixmap wKing(":/img/img/whiteKing.png");
    QPixmap bKing(":/img/img/blackKing.png");

    //棋盘中的方格的位子是要反一反的
    if(isInitial)
    {
        //isInitial = false;
        for(int i=0;i<8;i++)
        {
            for(int j=0;j<8;j++)
            {
                if(j==6)
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,wPawn);
                }
                if(j==1)
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,bPawn);
                }
                if(j==7 && (i==0||i==7))
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,wRook);
                }
                if(j==0 && (i==0||i==7))
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,bRook);
                }
                if(j==7 && (i==1||i==6))
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,wHorse);
                }
                if(j==0 && (i==1||i==6))
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,bHorse);
                }
                if(j==7 && (i==2||i==5))
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,wBishop);
                }
                if(j==0 && (i==2||i==5))
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,bBishop);
                }
                if(j==7 && (i==3))
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,wQueen);
                }
                if(j==0 && (i==3))
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,bQueen);
                }
                if(j==7 && (i==4))
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,wKing);
                }
                if(j==0 && (i==4))
                {
                    p.drawPixmap(i*unit,j*unit,unit,unit,bKing);
                }
            }
        }

    }

    p.end();
}

void ChessClient::initial()
{
        for(int i=0;i<8;i++)
        {
            matrix[i][6]=1;
            matrix[i][6]=-1;
        }
            matrix[0][7]=2;
            matrix[7][7]=2;
            matrix[0][0]=-2;
            matrix[7][0]=-2;
            matrix[1][7]=3;
            matrix[6][7]=3;
            matrix[1][0]=-3;
            matrix[6][0]=-3;
            matrix[2][7]=4;
            matrix[5][7]=4;
            matrix[2][0]=-4;
            matrix[5][0]=-4;
            matrix[3][7]=5;
            matrix[3][0]=-5;
            matrix[4][7]=6;
            matrix[4][0]=-6;
}
