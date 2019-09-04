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

    port =6666;



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
                qDebug()<<"客户端已连接";
                menu->setEnabled(false);

            });

        });
       connect(tcpClientSocket,&QTcpSocket::disconnected,
               [=]()
       {
           qDebug()<<"客户端已断开";

           if(nullptr == tcpClientSocket)
           {
               return;
           }
           menu->setEnabled(true);
           //tcpClientSocket->disconnectFromHost();
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
                matrix[i][j]=false;
            }
        }


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

    p.end();
}
