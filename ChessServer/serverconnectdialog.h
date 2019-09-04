#ifndef SERVERCONNECTDIALOG_H
#define SERVERCONNECTDIALOG_H

#include <QDialog>
#include<QDebug>
#include<QMessageBox>
#include<QCloseEvent>

namespace Ui {
class serverConnectDialog;
}

class serverConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit serverConnectDialog(QWidget *parent = nullptr);
    ~serverConnectDialog();

signals:
    void canConnect(QString);
    void cannotConnect();

private:
    Ui::serverConnectDialog *ui;

    bool haveToClose;

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // SERVERCONNECTDIALOG_H
