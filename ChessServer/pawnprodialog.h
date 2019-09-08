#ifndef PAWNPRODIALOG_H
#define PAWNPRODIALOG_H

#include <QDialog>

#include<QDebug>
#include<QMessageBox>
#include<QCloseEvent>

namespace Ui {
class pawnProDialog;
}

class pawnProDialog : public QDialog
{
    Q_OBJECT

public:
    explicit pawnProDialog(QWidget *parent = nullptr);
    ~pawnProDialog();

signals:
    void toQueen();
    void toBishop();
    void toHorse();
    void toRook();

private:
    Ui::pawnProDialog *ui;

protected:



};

#endif // PAWNPRODIALOG_H
