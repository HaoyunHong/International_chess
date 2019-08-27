#include "client.h"
#include "ui_client.h"

Client::Client(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Client)
{
    ui->setupUi(this);

    this->setFixedSize(1500,900);

    this->setWindowTitle("Client [Black]");

    QIcon mainIcon(":/img/img/black_queen.png");
    this->setWindowIcon(mainIcon);

        for(int i=0;i<8;i++)
        {
            for(int j=0;j<8;j++)
            {
                matrix[i][j]=false;
            }
        }
}

Client::~Client()
{
    delete ui;
}

void Client::paintEvent(QPaintEvent *e)
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
    p.drawPixmap(30,30,840,840,frame);

    QRect rec = QRect(50,50,800,800);
    QPixmap back(":/img/img/draw.jpg");
    p.drawPixmap(rec,back);

    int unit = 100;

    p.translate(50,50);

    QPen pen;
    pen.setColor(QColor(106,64,40));

    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    for(int i=1;i<8;i++)
    {
        p.drawLine(i*unit,0,i*unit,800);
        p.drawLine(0,i*unit,800,i*unit);
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
