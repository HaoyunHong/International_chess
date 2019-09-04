#ifndef CLIENTCONNECTDIALOG_H
#define CLIENTCONNECTDIALOG_H

#include <QDialog>
#include<QDebug>
#include<QMessageBox>

namespace Ui {
class clientConnectDialog;
}

class clientConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit clientConnectDialog(QWidget *parent = nullptr);
    ~clientConnectDialog();

signals:
    void canConnect(QString);

private:
    Ui::clientConnectDialog *ui;
    bool isValid(QString str);
    QString ip;
};

#endif // CLIENTCONNECTDIALOG_H
