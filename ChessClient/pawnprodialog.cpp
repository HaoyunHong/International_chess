#include "pawnprodialog.h"
#include "ui_pawnprodialog.h"

pawnProDialog::pawnProDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::pawnProDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("Please choose one chess to promote your soldier!");

    int queenCnt = 0;
    int bishopCnt = 0;
    int horseCnt = 0;
    int rookCnt = 0;

    connect(ui->queenButton,&QPushButton::clicked,
            [=]()mutable
    {
        queenCnt++;
        if(queenCnt==1)
        {
            this->setWindowTitle("If sure, please click again!");
        }
        else {
            this->setWindowTitle("You choose Queen!");
            emit toQueen();
            qDebug()<<"emit toQueen()";
            haveToClose = true;
            this->close();
        }


    });
    connect(ui->bishopButton,&QPushButton::clicked,
            [=]()mutable
    {
        bishopCnt++;
        if(bishopCnt==1)
        {
            this->setWindowTitle("If sure, please click again!");
        }
        else {
            this->setWindowTitle("You choose Bishop!");
            emit toBishop();
            qDebug()<<"emit toBishop()";
            haveToClose = true;
            this->close();
        }
    });
    connect(ui->horseButton,&QPushButton::clicked,
            [=]()mutable
    {
        horseCnt++;
        if(horseCnt==1)
        {
            this->setWindowTitle("If sure, please click again!");
        }
        else {
            this->setWindowTitle("You choose Horse!");
            emit toHorse();
            qDebug()<<"emit toHorse()";
            haveToClose = true;
            this->close();
        }
    });
    connect(ui->rookButton,&QPushButton::clicked,
            [=]()mutable
    {
        rookCnt++;
        if(rookCnt==1)
        {
            this->setWindowTitle("If sure, please click again!");
        }
        else {
            this->setWindowTitle("You choose Rook!");
            emit toRook();
            qDebug()<<"emit toRook()";
            haveToClose = true;
            this->close();
        }
    });

    haveToClose = false;


}

pawnProDialog::~pawnProDialog()
{
    delete ui;
}


