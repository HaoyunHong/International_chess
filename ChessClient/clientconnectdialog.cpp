#include "clientconnectdialog.h"
#include "ui_clientconnectdialog.h"

clientConnectDialog::clientConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::clientConnectDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("Connecting to Host");

    connect(ui->okButton,&QPushButton::clicked,
            [=]()
    {
//        qDebug()<<"ui->ipLineEdit1->text(): "<<ui->ipLineEdit1->text();
//        qDebug()<<"ui->ipLineEdit2->text(): "<<ui->ipLineEdit2->text();
//        qDebug()<<"ui->ipLineEdit3->text(): "<<ui->ipLineEdit3->text();
//        qDebug()<<"ui->ipLineEdit4->text(): "<<ui->ipLineEdit4->text();

        if(!isValid(ui->ipLineEdit1->text())||!isValid(ui->ipLineEdit2->text())||!isValid(ui->ipLineEdit3->text())||!isValid(ui->ipLineEdit4->text()))
        {
            int ret = QMessageBox::warning(this, "Error", "Invalid input", QMessageBox::Ok);
            switch (ret)
            {
            case QMessageBox::Ok:
                break;
            default:
                break;
            }
            ui->ipLineEdit1->clear();
            ui->ipLineEdit2->clear();
            ui->ipLineEdit3->clear();
            ui->ipLineEdit4->clear();
        }
        else {
            ip = ui->ipLineEdit1->text()+"."+ui->ipLineEdit2->text()+"."+ui->ipLineEdit3->text()+"."+ui->ipLineEdit4->text();;
//            qDebug()<<"In client Ip = "<<ip;

            emit canConnect(ip);

            this->close();
        }
    });
}

clientConnectDialog::~clientConnectDialog()
{
    delete ui;
}

bool clientConnectDialog::isValid(QString str)
{
//    qDebug()<<"str = "<<str;
//    qDebug()<<"str.toInt(): "<<str.toInt();
    if(str.isEmpty() || str.toInt()<0 || str.toInt()>255)
    {
        return false;
    }
    return true;
}
