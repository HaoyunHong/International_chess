#include "serverconnectdialog.h"
#include "ui_serverconnectdialog.h"

serverConnectDialog::serverConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::serverConnectDialog)
{
    ui->setupUi(this);

    this->setWindowTitle("Listening");

    haveToClose = false;

    //ui->cancelButton->setEnabled(false);

    connect(ui->okButton,&QPushButton::clicked,
            [=]()
    {
        //ui->cancelButton->setEnabled(true);
        emit canConnect(ui->hostLineEdit->text());
        this->close();

    });

    connect(ui->cancelButton,&QPushButton::clicked,
            [=]()
    {
        emit cannotConnect();
        haveToClose = true;
        this->close();
    });
}

serverConnectDialog::~serverConnectDialog()
{
    delete ui;
}

void serverConnectDialog::closeEvent(QCloseEvent *event)
{
    if (haveToClose) return;
    int ret = QMessageBox::question(this, "question", "Do you want to continue the connection?", QMessageBox::Yes | QMessageBox::No);
    switch (ret)
    {
    case QMessageBox::Yes:
        ui->okButton->setEnabled(false);
        break;
    case QMessageBox::No:
        event->ignore();
        break;
    default:
        break;
    }
}
