#include "chessclient.h"
#include "ui_chessclient.h"

ChessClient::ChessClient(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChessClient)
{
    ui->setupUi(this);
    this->setFixedSize(500, 700);

    this->setWindowTitle("Client");

    ui->yourTurnlabel->setStyleSheet("color:black;background-color:red;");
    ui->yourTurnlabel->hide();

    isStart = false;

    //菜单栏
    mBar = menuBar();
    setMenuBar(mBar);

    //添加菜单
    menu = mBar->addMenu("Option");
    actClient = menu->addAction("Connect");

    menu2 = mBar->addMenu("Game");
    actSave = menu2->addAction("save game");

    port = 888;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            matrix[i][j] = 0;
        }
    }

    isLoad = false;

    menu2->setEnabled(false);

    connect(actClient, &QAction::triggered,
        [=]()
    {
        tcpClientSocket = new QTcpSocket(this);

        clientConnectDialog *cCDlg = new clientConnectDialog(this);


        connect(cCDlg, &clientConnectDialog::canConnect,
            [=](QString ipp)
        {
            ip = ipp;

            qDebug() << "Client: ip = " << ip;


            tcpClientSocket->connectToHost(QHostAddress(ip), port);

            connect(tcpClientSocket, &QTcpSocket::connected,
                [=]()
            {
                this->setWindowTitle("Client is connected to server");
                qDebug() << "客户端已连接";
                menu->setEnabled(false);
                menu2->setEnabled(true);

                connect(tcpClientSocket, &QTcpSocket::readyRead,
                    [=]()
                {
                    //这里直接统一一下格式
                    //因为怕粘的包的是不完整的
                    quint16 nextBlockSize = 0;
                    QDataStream in(tcpClientSocket);

                    in.setVersion(QDataStream::Qt_4_3);
                    forever{
                        int oX,oY,tX,tY;

                        in >> nextBlockSize;
                        qDebug() << "nextBlockSize: " << nextBlockSize;

                        if (nextBlockSize == 12345)
                        {
                            ui->yourTurnlabel->hide();
                            QString path;
                            in >> path;

                            if (!isLoad)
                            {
                                QString filePath = "Please load the draw file at " + path + " !";
                                int ret = QMessageBox::information(this, "Load Draw File",filePath , QMessageBox::Ok);

                                switch (ret)
                                {
                                    case QMessageBox::Ok:
                                    qDebug() << "open file";
                                    openFile();
                                    isLoad = true;
                                        break;
                                    default:
                                        break;
                                }
                            }

                            break;
                        }
                        if (nextBlockSize == 7777)
                        {
                            timerCount.stop();
                            int ret = QMessageBox::information(this, "Win", "[Opposite Give Up] You Win! Please wait for Server's reaction.", QMessageBox::Ok);
                            switch (ret)
                            {
                                case QMessageBox::Ok:
                                    this->setWindowTitle("Please wait for Server's reaction");
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        if (nextBlockSize == 8888)
                        {
                            timerCount.stop();
                            playAgain();
                        }
                        if (nextBlockSize == 6666)
                        {
                            timerCount.stop();
                            int ret = QMessageBox::information(this, "Win", "[Opposite Time Out] You Win! Please wait for Server's reaction.", QMessageBox::Ok);
                            switch (ret)
                            {
                                case QMessageBox::Ok:
                                    this->setWindowTitle("Please wait for Server's reaction");
                                    break;
                                default:
                                    break;
                            }
                            break;
                        }
                        if (nextBlockSize == 4)
                        {
                            in >> oX;
                            qDebug() << "Initial!";
                            initial();
                            update();
                            break;
                        }

                        //可能有好几个包
                        if (nextBlockSize == 0)
                        {
                            //这样说明in里面没东西
                            if (tcpClientSocket->bytesAvailable() < sizeof(quint16))
                            {
                                break;//比都有的信息的大小都小了，就说明没有包了
                            }
                            //in >> nextBlockSize;//要不然我们就可以获得后一个包的完整大小
                        }

                        if (tcpClientSocket->bytesAvailable() < nextBlockSize)
                        {
                            break;//说明后面的包不完整
                        }


                        in >> oX >> oY >> tX >> tY;
                            opposeOrigin = QPoint(oX,oY);
                            opposeTo = QPoint(tX,tY);
                            nextBlockSize = 0;//有些东西还是放在缓冲区里
                            qDebug() << "opposeOrigin = " << opposeOrigin;
                            qDebug() << "opposeTo = " << opposeTo;

                            matrix[tX][tY] = matrix[oX][oY];
                            matrix[oX][oY] = 0;

                            quint16 pro = 0;
                            in >> pro;
                            qDebug() << "in client pro: " << pro;
                            if (pro > 0 && pro < 6)
                            {
                                matrix[tX][tY] = pro;
                            }



                            qDebug() << "Client receive point:";
                            qDebug() << opposeOrigin;
                            qDebug() << opposeTo;

                            step++;
                            actSave->setEnabled(true);
                            qDebug() << "Before Client turn step = " << step;
                            update();
                            timerCount.start(1000);

                            if (pro == 505)
                            {
                                timerCount.stop();
                                int ret = QMessageBox::information(this, "Lose", "[Be Checkmated] You Lose!Please wait for Server's reaction.", QMessageBox::Ok);
                                switch (ret)
                                {
                                case QMessageBox::Ok:
                                    this->setWindowTitle("Please wait for Server's reaction");
                                    break;
                                default:
                                    this->setWindowTitle("Please wait for Server's reaction");
                                    break;
                                }
                            }
                    }

                });

                if (!isLose)
                {
                    connect(ui->giveUpButton, &QPushButton::clicked,
                        [=]()
                    {
                        isLose = true;
                        ui->lcdNumber->display(0);
                        timerCount.stop();
                        QByteArray block;
                        QDataStream out(&block, QIODevice::WriteOnly);
                        out.setVersion(QDataStream::Qt_4_3);
                        //先写一个0来给将要传出去的信息的大小数据占个位子
                        out << quint16(7777);
                        tcpClientSocket->write(block);

                        int ret = QMessageBox::information(this, "Lose", "[You Give Up] You Lose! Please wait for Server's reaction.", QMessageBox::Ok);
                        switch (ret)
                        {
                        case QMessageBox::Ok:
                            this->setWindowTitle("Please wait for Server's reaction");
                            break;
                        default:
                            break;
                        }

                    });

                }

            });

        });
        connect(tcpClientSocket, &QTcpSocket::disconnected,
            [=]()
        {
            isLose = false;
            ui->lcdNumber->display(0);
            this->setWindowTitle("Server canceled the connection");
            qDebug() << "客户端已断开";


            if (nullptr == tcpClientSocket)
            {
                return;
            }
            menu->setEnabled(true);
            menu2->setEnabled(false);
            actSave->setEnabled(true);

            tcpClientSocket->close();
            tcpClientSocket = nullptr;
            focus = QPoint(-1, -1);
            curLeftClick = QPoint(-1, -1);
            isSelected = false;
            hasDestination = false;

            for (int i = 0; i < 8; i++)
            {
                for (int j = 0; j < 8; j++)
                {
                    matrix[i][j] = 0;
                }
            }
            update();

            step = 0;
            ui->yourTurnlabel->hide();

            menu->setEnabled(true);
            menu2->setEnabled(false);

            timerCount.stop();
            drawLineIndex = 0;
            isStart = false;
            isLoad = false;

            countTime = 60;
            startTime = 3;
        });

        cCDlg->exec();

    });

    startTime = 3;
    connect(&timeStart, &QTimer::timeout,
        [=]()
    {
        ui->yourTurnlabel->hide();
        ui->lcdNumber->display(startTime);
        if (startTime == 0)
        {
            ui->yourTurnlabel->show();
            timerCount.start(1000);
            timeStart.stop();
        }
        startTime--;
    });

    countTime = 60;
    connect(&timerCount, &QTimer::timeout,
        [=]()
    {
        isStart = true;
        ui->lcdNumber->display(countTime);
        if (countTime == 0)
        {
            timerCount.stop();
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_3);
            //先写一个0来给将要传出去的信息的大小数据占个位子
            out << quint16(6666);
            tcpClientSocket->write(block);
            int ret = QMessageBox::information(this, "Lose", "[Time Out] You Lose! Please wait for Server's reaction.", QMessageBox::Ok);
            switch (ret)
            {
            case QMessageBox::Ok:
                this->setWindowTitle("Please wait for Server's reaction");
                break;
            default:
                break;
            }

        }
        countTime--;

    });

    QIcon mainIcon(":/img/img/blackQueen.png");
    this->setWindowIcon(mainIcon);

    focus = QPoint(-1, -1);
    curLeftClick = QPoint(-1, -1);
    menu2->setEnabled(false);
    isSelected = false;
    hasDestination = false;

    step = 0;

    drawLineIndex = 0;

    actSave->setEnabled(false);

    isLose = false;
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
    p.setRenderHints(QPainter::SmoothPixmapTransform);//消锯Pixmap的锯齿

    QBrush brush;
    brush.setColor(QColor(250, 234, 211));
    brush.setStyle(Qt::SolidPattern);

    QPixmap wood(":/img/img/wood.jpg");
    p.drawPixmap(0, 0, width(), height(), wood);

    QPixmap frame(":/img/img/kuang.jpg");
    p.drawPixmap(40, 70, 420, 420, frame);

    QRect rec = QRect(50, 80, 400, 400);
    QPixmap back(":/img/img/draw.jpg");
    p.drawPixmap(rec, back);

    int unit = 50;

    p.translate(50, 80);

    QPen pen;
    pen.setColor(QColor(106, 64, 40));

    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    for (int i = 1; i < 8; i++)
    {
        p.drawLine(i*unit, 0, i*unit, 400);
        p.drawLine(0, i*unit, 400, i*unit);
    }

    brush.setColor(QColor(106, 64, 40));
    //brush.setStyle(Qt::SolidPattern);
    p.setBrush(brush);

    QPixmap backb(":/img/img/backb.jpg");
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if ((i + j) % 2 == 1)
            {
                p.drawPixmap(i*unit, j*unit, unit, unit, backb);
            }
        }
    }

    if (step % 2 == 1)
    {
        if (isStart)
        {
            ui->yourTurnlabel->show();
        }

        if (curLeftClick != QPoint(-1, -1))
        {
            //qDebug()<<"curLeftClick = "<<curLeftClick;
    //        for(int i=0;i<8;i++)
    //        {
    //            for(int j=0;j<8;j++)
    //            {
    //                qDebug()<<"matrix["<<i<<"]["<<j<<"]: "<<matrix[i][j];
    //            }
    //        }
            brush.setColor(QColor(255, 255, 255, 180));
            p.setPen(Qt::NoPen);
            p.setBrush(brush);


            //        for(int i=0;i<curClickPath.size();i++)
            //        {
            //            qDebug()<<"curClickPath["<<i<<"]: "<<curClickPath[i];
            //        }
                   // qDebug()<<"Here!";

            for (int i = 0; i < curClickPath.size(); i++)
            {
                int x = curClickPath[i].x();
                int y = curClickPath[i].y();
                p.drawRect(x*unit, y*unit, unit, unit);
            }
            p.drawRect(curLeftClick.x()*unit, curLeftClick.y()*unit, unit, unit);
            curClickPath.clear();
        }

        if (focus != QPoint(-1, -1))
        {
            pen.setColor(QColor(255, 0, 0, 200));
            pen.setWidth(3);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);

            for (int i = 0; i < focusPath.size(); i++)
            {
                int x = focusPath[i].x();
                int y = focusPath[i].y();
                p.drawRect(x*unit, y*unit, unit, unit);
            }
            p.drawRect(focus.x()*unit, focus.y()*unit, unit, unit);
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


    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (matrix[i][j] == 1)
                p.drawPixmap(i*unit, j*unit, unit, unit, wPawn);

            if (matrix[i][j] == -1)
                p.drawPixmap(i*unit, j*unit, unit, unit, bPawn);

            if (matrix[i][j] == 2)
                p.drawPixmap(i*unit, j*unit, unit, unit, wRook);

            if (matrix[i][j] == -2)
                p.drawPixmap(i*unit, j*unit, unit, unit, bRook);

            if (matrix[i][j] == 3)
                p.drawPixmap(i*unit, j*unit, unit, unit, wHorse);

            if (matrix[i][j] == -3)
                p.drawPixmap(i*unit, j*unit, unit, unit, bHorse);

            if (matrix[i][j] == 4)
                p.drawPixmap(i*unit, j*unit, unit, unit, wBishop);

            if (matrix[i][j] == -4)
                p.drawPixmap(i*unit, j*unit, unit, unit, bBishop);

            if (matrix[i][j] == 5)
                p.drawPixmap(i*unit, j*unit, unit, unit, wQueen);

            if (matrix[i][j] == -5)
                p.drawPixmap(i*unit, j*unit, unit, unit, bQueen);

            if (matrix[i][j] == 6)
                p.drawPixmap(i*unit, j*unit, unit, unit, wKing);

            if (matrix[i][j] == -6)
                p.drawPixmap(i*unit, j*unit, unit, unit, bKing);

        }
    }


    p.end();
}

void ChessClient::initial()
{
    for (int i = 0; i < 8; i++)
    {
        matrix[i][6] = 1;
        matrix[i][1] = -1;
    }
    matrix[0][7] = 2;
    matrix[7][7] = 2;
    matrix[0][0] = -2;
    matrix[7][0] = -2;
    matrix[1][7] = 3;
    matrix[6][7] = 3;
    matrix[1][0] = -3;
    matrix[6][0] = -3;
    matrix[2][7] = 4;
    matrix[5][7] = 4;
    matrix[2][0] = -4;
    matrix[5][0] = -4;
    matrix[3][7] = 5;
    matrix[3][0] = -5;
    matrix[4][7] = 6;
    matrix[4][0] = -6;
}

void ChessClient::mousePressEvent(QMouseEvent *e)
{
    if (step % 2 != 1 || !isStart)
    {
        return;
    }

    QPoint curPoint = e->pos();
    QPoint centerIJ;

    int unit = 50;

    QPoint o(50, 80);

    if (e->button() == Qt::LeftButton)
    {
        if (isSelected)
        {
            curLeftClick = QPoint(-1, -1);
            return;
        }

        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                centerIJ.setX(i*unit + unit / 2);
                centerIJ.setY(j*unit + unit / 2);
                QRect rec = QRect(o.x() + centerIJ.x() - unit / 2, o.y() + centerIJ.y() - unit / 2, unit, unit);
                if (rec.contains(curPoint) && matrix[i][j] < 0)
                {
                    curLeftClick = QPoint(i, j);
                    qDebug() << "curLeftClick: " << curLeftClick;
                    //然后这个时候在点击的时候就会有路径提示
                    setMovePoints(curLeftClick);

                    update();
                    //curClickPath.clear();
                }
            }
        }
    }

    if (e->button() == Qt::RightButton)
    {
        actSave->setEnabled(true);
        //qDebug()<<"Here!";
        //qDebug()<<"focusPath.size()"<<focusPath.size();
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                centerIJ.setX(i*unit + unit / 2);
                centerIJ.setY(j*unit + unit / 2);
                QRect rec = QRect(o.x() + centerIJ.x() - unit / 2, o.y() + centerIJ.y() - unit / 2, unit, unit);
                if (rec.contains(curPoint) && focusPath.contains(QPoint(i, j)))
                {
                    actSave->setEnabled(true);
                    hasDestination = true;

                    bool isEating = false;
                    if (matrix[i][j] == 6)
                    {
                        isEating = true;
                    }

                    matrix[i][j] = matrix[focus.x()][focus.y()];
                    matrix[focus.x()][focus.y()] = 0;

                    update();


                    if (isEating)
                    {
                        QByteArray block;
                        QDataStream out(&block, QIODevice::WriteOnly);
                        out.setVersion(QDataStream::Qt_4_3);
                        //先写一个0来给将要传出去的信息的大小数据占个位子
                        out << quint16(0) << focus.x() << focus.y() << i << j << quint16(505);
                        //找到那个0，再覆盖掉
                        out.device()->seek(0);
                        out << quint16(block.size() - sizeof(quint16));
                        tcpClientSocket->write(block);

                        qDebug() << "Server send:";
                        qDebug() << quint16(block.size() - sizeof(quint16));
                        qDebug() << focus;
                        qDebug() << i << "," << j;

                        int ret = QMessageBox::information(this, "Win", "[Checkmate] You Win! Please wait for Server's reaction.", QMessageBox::Ok);
                        switch (ret)
                        {
                        case QMessageBox::Ok:
                            this->setWindowTitle("Please wait for Server's reaction");
                            break;
                        default:
                            this->setWindowTitle("Please wait for Server's reaction");
                            break;
                        }

                        return;
                    }

                    //如果此时满足兵升变的条件
                    qDebug() << "j = " << j;
                    if (j == 7 && matrix[i][j] == -1)
                    {
                        qDebug() << "j = " << j;
                        pdlg = new pawnProDialog(this);

                        connect(pdlg, &pawnProDialog::toQueen,
                            [=]()
                        {
                            matrix[i][j] = 5;
                            qDebug() << "toQueen";

                        });
                        connect(pdlg, &pawnProDialog::toBishop,
                            [=]()
                        {
                            matrix[i][j] = 4;
                        });
                        connect(pdlg, &pawnProDialog::toHorse,
                            [=]()
                        {
                            matrix[i][j] = 3;
                        });
                        connect(pdlg, &pawnProDialog::toRook,
                            [=]()
                        {
                            matrix[i][j] = 2;
                        });

                        pdlg->exec();

                        update();

                        QByteArray block;
                        QDataStream out(&block, QIODevice::WriteOnly);
                        out.setVersion(QDataStream::Qt_4_3);
                        //先写一个0来给将要传出去的信息的大小数据占个位子
                        out << quint16(0) << focus.x() << focus.y() << i << j << quint16(matrix[i][j]);
                        //找到那个0，再覆盖掉
                        out.device()->seek(0);
                        out << quint16(block.size() - sizeof(quint16));
                        tcpClientSocket->write(block);

                        qDebug() << "Client send:";
                        qDebug() << quint16(block.size() - sizeof(quint16));
                        qDebug() << focus;
                        qDebug() << i << "," << j;
                    }
                    else {
                        QByteArray block;
                        QDataStream out(&block, QIODevice::WriteOnly);
                        out.setVersion(QDataStream::Qt_4_3);
                        //先写一个0来给将要传出去的信息的大小数据占个位子
                        out << quint16(0) << focus.x() << focus.y() << i << j << quint16(0);
                        //找到那个0，再覆盖掉
                        out.device()->seek(0);
                        out << quint16(block.size() - sizeof(quint16));
                        tcpClientSocket->write(block);

                        qDebug() << "Client send:";
                        qDebug() << quint16(block.size() - sizeof(quint16));
                        qDebug() << focus;
                        qDebug() << i << "," << j;
                    }



                    update();
                    timerCount.stop();
                    countTime = 60;
                    ui->lcdNumber->display(60);

                    step++;
                    qDebug() << "This Client turn step = " << step;
                    focusPath.clear();
                    focus = QPoint(-1, -1);
                    isSelected = false;
                    ui->yourTurnlabel->hide();
                }
            }
        }
    }
}

void ChessClient::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (step % 2 != 1 || !isStart)
    {
        return;
    }
    if (isSelected) return;
    QPoint curPoint = e->pos();
    QPoint centerIJ;

    int unit = 50;

    QPoint o(50, 80);
    if (e->button() == Qt::LeftButton)
    {
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                centerIJ.setX(i*unit + unit / 2);
                centerIJ.setY(j*unit + unit / 2);
                QRect rec = QRect(o.x() + centerIJ.x() - unit / 2, o.y() + centerIJ.y() - unit / 2, unit, unit);

                //这个括号里还要再加些条件
                if (rec.contains(curPoint) && matrix[i][j] != 0)
                {
                    isSelected = true;

                    focus = QPoint(i, j);
                    setMovePoints(focus);
                    if (focusPath.isEmpty())
                    {
                        isSelected = false;
                        focus = QPoint(-1, -1);
                        focusPath.clear();
                        qDebug() << "can't move!";
                    }
                    else {
                        //就是表示这个追踪我不用了
                        curLeftClick = QPoint(-1, -1);
                        qDebug() << "In Server: focus = " << focus;
                    }

                    update();
                }
            }
        }

    }
}

QPixmap ChessClient::getPic(QPoint p)
{
    int i = p.x();
    int j = p.y();

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

    QPixmap pic;
    switch (matrix[i][j]) {
    case 0:
        break;
    case 1:
        pic = wPawn;
        break;
    case -1:
        pic = bPawn;
        break;
    case 2:
        pic = wRook;
        break;
    case -2:
        pic = bRook;
        break;
    case 3:
        pic = wHorse;
        break;
    case -3:
        pic = bHorse;
        break;
    case 4:
        pic = wBishop;
        break;
    case -4:
        pic = bBishop;
        break;
    case 5:
        pic = wQueen;
        break;
    case -5:
        pic = bQueen;
        break;
    case 6:
        pic = wKing;
        break;
    case -6:
        pic = bKing;
        break;
    default:
        break;
    }

    return pic;
}

void ChessClient::setMovePoints(QPoint curClick)
{
    int x = curClick.x();
    int y = curClick.y();

    //这里要注意的是兵升变的时候不能不变
    if (matrix[x][y] == -1)
    {
        qDebug() << "bPawn!";
        QPoint p(x, y + 1);
        if (matrix[x][y + 1] == 0)
        {
            curClickPath.push_back(p);
            if (y == 1)
            {
                curClickPath.push_back(QPoint(x, 3));
            }
        }
        if (x > 0 && y < 7 && matrix[x - 1][y + 1]>0)
        {
            curClickPath.push_back(QPoint(x - 1, y + 1));
        }
        if (x < 7 && y < 7 && matrix[x + 1][y + 1]>0)
        {
            curClickPath.push_back(QPoint(x + 1, y + 1));
        }

        //        for(int i=0;i<curClickPath.size();i++)
        //        {
        //            if(matrix[curClickPath[i].x()][curClickPath[i].y()]!=0)
        //            {
        //                curClickPath.remove(i);
        //            }
        //        }
    }

    if (matrix[x][y] == -2)
    {
        qDebug() << "wRook!";

        //        for(int i=0;i<8;i++)
        //        {
        //            for(int j=0;j<8;j++)
        //            {
        //                qDebug() << "matrix["<<i<<"]["<<j<<"]: "<<matrix[i][j];
        //            }
        //        }
        if (x < 7)
        {
            for (int i = x + 1; i < 8; i++)
            {
                if (matrix[i][y] == 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                }
                if (matrix[i][y] > 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                    break;
                }
                if (matrix[i][y] < 0)
                {
                    break;
                }
            }
        }

        if (x > 0)
        {
            for (int i = x - 1; i >= 0; i--)
            {
                if (matrix[i][y] == 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                }
                if (matrix[i][y] > 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                    break;
                }
                if (matrix[i][y] < 0)
                {
                    break;
                }
            }
        }

        if (y < 7)
        {
            for (int j = y + 1; j < 8; j++)
            {
                if (matrix[x][j] == 0)
                {
                    curClickPath.push_back(QPoint(x, j));
                }
                if (matrix[x][j] < 0)
                {
                    break;
                }
                if (matrix[x][j] > 0)
                {
                    curClickPath.push_back(QPoint(x, j));
                    break;
                }
            }
        }

        if (y > 0)
        {
            for (int j = y - 1; j >= 0; j--)
            {
                if (matrix[x][j] == 0)
                {
                    curClickPath.push_back(QPoint(x, j));
                }
                if (matrix[x][j] < 0)
                {
                    break;
                }
                if (matrix[x][j] > 0)
                {
                    curClickPath.push_back(QPoint(x, j));
                    break;
                }
            }
        }

        //        for(int i=0;i<curClickPath.size();i++)
        //        {
        //            if(matrix[curClickPath[i].x()][curClickPath[i].y()]>0)
        //            {
        //                curClickPath.remove(i);
        //            }
        //        }

    }

    if (matrix[x][y] == -3)
    {
        qDebug() << "bHorse!";
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if ((i - x)*(i - x) + (j - y)*(j - y) == 5 && matrix[i][j] >= 0)
                {
                    curClickPath.push_back(QPoint(i, j));
                }
            }
        }

        //        for(int i=0;i<curClickPath.size();i++)
        //        {
        //            if(matrix[curClickPath[i].x()][curClickPath[i].y()]>0)
        //            {
        //                curClickPath.remove(i);
        //            }
        //        }
    }

    if (matrix[x][y] == -4)
    {
        qDebug() << "bBishop!";
        int _x = x;
        int _y = y;
        while (_x > 0 && _y > 0)
        {
            _x--;
            _y--;
            if (matrix[_x][_y] == 0)
            {
                curClickPath.push_back(QPoint(_x, _y));
            }
            else if (matrix[_x][_y] < 0)
            {
                break;
            }
            else if (matrix[_x][_y] > 0)
            {
                curClickPath.push_back(QPoint(_x, _y));
                break;
            }
        }
        int x_ = x;
        int y_ = y;
        while (x_ < 7 && y_ < 7)
        {
            x_++;
            y_++;
            if (matrix[x_][y_] == 0)
            {
                curClickPath.push_back(QPoint(x_, y_));
            }
            else if (matrix[x_][y_] < 0)
            {
                break;
            }
            else if (matrix[x_][y_] > 0)
            {
                curClickPath.push_back(QPoint(x_, y_));
                break;
            }
        }
        int __x = x;
        int __y = y;
        while (__x > 0 && __y < 7)
        {
            __x--;
            __y++;
            if (matrix[__x][__y] == 0)
            {
                curClickPath.push_back(QPoint(__x, __y));
            }
            else if (matrix[__x][__y] < 0)
            {
                break;
            }
            else if (matrix[__x][__y] > 0)
            {
                curClickPath.push_back(QPoint(__x, __y));
                break;
            }
        }
        int x__ = x;
        int y__ = y;
        while (x__ < 7 && y__>0)
        {
            x__++;
            y__--;
            if (matrix[x__][y__] == 0)
            {
                curClickPath.push_back(QPoint(x__, y__));
            }
            else if (matrix[x__][y__] < 0)
            {
                break;
            }
            else if (matrix[x__][y__] > 0)
            {
                curClickPath.push_back(QPoint(x__, y__));
                break;
            }
        }

        //        for(int i=0;i<curClickPath.size();i++)
        //        {
        //            if(matrix[curClickPath[i].x()][curClickPath[i].y()]>0)
        //            {
        //                curClickPath.remove(i);
        //            }
        //        }
    }

    if (matrix[x][y] == -5)
    {
        qDebug() << "bQueen!";
        if (x < 7)
        {
            for (int i = x + 1; i < 8; i++)
            {
                if (matrix[i][y] == 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                }
                if (matrix[i][y] > 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                    break;
                }
                if (matrix[i][y] < 0)
                {
                    break;
                }
            }
        }

        if (x > 0)
        {
            for (int i = x - 1; i >= 0; i--)
            {
                if (matrix[i][y] == 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                }
                if (matrix[i][y] > 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                    break;
                }
                if (matrix[i][y] < 0)
                {
                    break;
                }
            }
        }

        if (y < 7)
        {
            for (int j = y + 1; j < 8; j++)
            {
                if (matrix[x][j] == 0)
                {
                    curClickPath.push_back(QPoint(x, j));
                }
                if (matrix[x][j] < 0)
                {
                    break;
                }
                if (matrix[x][j] > 0)
                {
                    curClickPath.push_back(QPoint(x, j));
                    break;
                }
            }
        }

        if (y > 0)
        {
            for (int j = y - 1; j >= 0; j--)
            {
                if (matrix[x][j] == 0)
                {
                    curClickPath.push_back(QPoint(x, j));
                }
                if (matrix[x][j] < 0)
                {
                    break;
                }
                if (matrix[x][j] > 0)
                {
                    curClickPath.push_back(QPoint(x, j));
                    break;
                }
            }
        }


        int _x = x;
        int _y = y;
        while (_x > 0 && _y > 0)
        {
            _x--;
            _y--;
            if (matrix[_x][_y] == 0)
            {
                curClickPath.push_back(QPoint(_x, _y));
            }
            else if (matrix[_x][_y] < 0)
            {
                break;
            }
            else if (matrix[_x][_y] > 0)
            {
                curClickPath.push_back(QPoint(_x, _y));
                break;
            }
        }
        int x_ = x;
        int y_ = y;
        while (x_ < 7 && y_ < 7)
        {
            x_++;
            y_++;
            if (matrix[x_][y_] == 0)
            {
                curClickPath.push_back(QPoint(x_, y_));
            }
            else if (matrix[x_][y_] < 0)
            {
                break;
            }
            else if (matrix[x_][y_] > 0)
            {
                curClickPath.push_back(QPoint(x_, y_));
                break;
            }
        }
        int __x = x;
        int __y = y;
        while (__x > 0 && __y < 7)
        {
            __x--;
            __y++;
            if (matrix[__x][__y] == 0)
            {
                curClickPath.push_back(QPoint(__x, __y));
            }
            else if (matrix[__x][__y] < 0)
            {
                break;
            }
            else if (matrix[__x][__y] > 0)
            {
                curClickPath.push_back(QPoint(__x, __y));
                break;
            }
        }
        int x__ = x;
        int y__ = y;
        while (x__ < 7 && y__>0)
        {
            x__++;
            y__--;
            if (matrix[x__][y__] == 0)
            {
                curClickPath.push_back(QPoint(x__, y__));
            }
            else if (matrix[x__][y__] < 0)
            {
                break;
            }
            else if (matrix[x__][y__] > 0)
            {
                curClickPath.push_back(QPoint(x__, y__));
                break;
            }
        }

        //        for(int i=0;i<curClickPath.size();i++)
        //        {
        //            if(matrix[curClickPath[i].x()][curClickPath[i].y()]>0)
        //            {
        //                curClickPath.remove(i);
        //            }
        //        }
    }

    if (matrix[x][y] == -6)
    {
        qDebug() << "bKing!";
        if (x > 0 && matrix[x - 1][y] >= 0)
        {
            curClickPath.push_back(QPoint(x - 1, y));
        }
        if (x > 0 && y > 0 && matrix[x - 1][y - 1] >= 0)
        {
            curClickPath.push_back(QPoint(x - 1, y - 1));
        }
        if (x > 0 && y < 7 && matrix[x - 1][y + 1] >= 0)
        {
            curClickPath.push_back(QPoint(x - 1, y + 1));
        }
        if (y > 0 && matrix[x][y - 1] >= 0)
        {
            curClickPath.push_back(QPoint(x, y - 1));
        }
        if (y < 7 && matrix[x][y + 1] >= 0)
        {
            curClickPath.push_back(QPoint(x, y + 1));
        }
        if (x < 7 && y>0 && matrix[x + 1][y - 1] >= 0)
        {
            curClickPath.push_back(QPoint(x + 1, y - 1));
        }
        if (x < 7 && matrix[x + 1][y] >= 0)
        {
            curClickPath.push_back(QPoint(x + 1, y));
        }
        if (x < 7 && y < 7 && matrix[x + 1][y + 1] >= 0)
        {
            curClickPath.push_back(QPoint(x + 1, y + 1));
        }
    }


    for (int i = 0; i < curClickPath.size(); i++)
    {
        qDebug() << "curClickPath[" << i << "]: " << curClickPath[i];
    }
    if (isSelected)
    {
        focusPath = curClickPath;
        curClickPath.clear();
    }
}

void ChessClient::playAgain()
{
    isLose = false;
    actSave->setEnabled(false);
    timerCount.stop();
    drawLineIndex = 0;
    isStart = false;
    isLoad = false;
    //这里就是重新来，不用再连接
    ui->yourTurnlabel->hide();
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            matrix[i][j] = 0;
        }
    }

    focus = QPoint(-1, -1);
    curLeftClick = QPoint(-1, -1);
    isSelected = false;
    step = 0;
    countTime = 60;

    update();

    menu->setEnabled(false);
    actSave->setEnabled(true);
    ui->lcdNumber->display(0);

}

void ChessClient::fileParser(QByteArray array)
{

    QString str(array);
    qDebug() << "In fileParser: " << str;
    QStringList strList = str.split(" ");

    int num = 0;

    QPoint* chessPos;

    qDebug() << "drawLineIndex = " << drawLineIndex;

    if (drawLineIndex == 1)
    {
        if (strList[0].contains("white"))
        {
            qDebug() << "white first!";
            step = 0;
            isMine = false;
        }
        else if (strList[0].contains("black"))
        {
            qDebug() << "black first!";
            step = 1;
            isMine = true;
        }

    }
    else {
        if (strList[0].contains("white"))
        {
            qDebug() << "white second!";
            isMine = false;
        }
        else if (strList[0].contains("black"))
        {
            qDebug() << "black second!";
            isMine = true;
        }

        if (strList.size() > 2)
        {
            num = strList[1].toInt();
            chessPos = new QPoint[num];

            for (int i = 2; i < num + 2; i++)
            {
                if (strList[i][0] == "a")
                {
                    chessPos[i - 2].setX(0);
                }
                if (strList[i][0] == "b")
                {
                    chessPos[i - 2].setX(1);
                }
                if (strList[i][0] == "c")
                {
                    chessPos[i - 2].setX(2);
                }
                if (strList[i][0] == "d")
                {
                    chessPos[i - 2].setX(3);
                }
                if (strList[i][0] == "e")
                {
                    chessPos[i - 2].setX(4);
                }
                if (strList[i][0] == "f")
                {
                    chessPos[i - 2].setX(5);
                }
                if (strList[i][0] == "g")
                {
                    chessPos[i - 2].setX(6);
                }
                if (strList[i][0] == "h")
                {
                    chessPos[i - 2].setX(7);
                }
                if (strList[i][1] == "1")
                {
                    chessPos[i - 2].setY(7);
                }
                if (strList[i][1] == "2")
                {
                    chessPos[i - 2].setY(6);
                }
                if (strList[i][1] == "3")
                {
                    chessPos[i - 2].setY(5);
                }
                if (strList[i][1] == "4")
                {
                    chessPos[i - 2].setY(4);
                }
                if (strList[i][1] == "5")
                {
                    chessPos[i - 2].setY(3);
                }
                if (strList[i][1] == "6")
                {
                    chessPos[i - 2].setY(2);
                }
                if (strList[i][1] == "7")
                {
                    chessPos[i - 2].setY(1);
                }
                if (strList[i][1] == "8")
                {
                    chessPos[i - 2].setY(0);
                }
            }

            qDebug() << "isMine: " << isMine;
            if (strList[0] == "king")
            {
                for (int i = 0; i < num; i++)
                {
                    if (isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()] = -6;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()] = 6;
                    }
                }
            }
            if (strList[0] == "queen")
            {
                for (int i = 0; i < num; i++)
                {
                    if (isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()] = -5;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()] = 5;
                    }
                }
            }
            if (strList[0] == "bishop")
            {
                for (int i = 0; i < num; i++)
                {
                    if (isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()] = -4;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()] = 4;
                    }
                }
            }
            if (strList[0] == "horse")
            {
                for (int i = 0; i < num; i++)
                {
                    if (isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()] = -3;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()] = 3;
                    }
                }
            }
            if (strList[0] == "rook")
            {
                for (int i = 0; i < num; i++)
                {
                    if (isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()] = -2;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()] = 2;
                    }
                }
            }
            if (strList[0] == "pawn")
            {
                for (int i = 0; i < num; i++)
                {
                    if (isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()] = -1;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()] = 1;
                    }
                }
            }
        }


        for (int j = 0; j < num; j++)
        {
            qDebug() << drawLineIndex << ": chessPos[" << j << "]" << chessPos[j];
        }
    }

    qDebug() << "In client fileParser step: " << step;
}

void ChessClient::openFile()
{
    QString path = QFileDialog::getOpenFileName(this,
        "[Client] Please choose a draw file", "../tests/", "TXT(*.txt)");
    //只有当文件不为空时才进行操作
    if (path.isEmpty() == false)
    {
        //文件操作
        QFile file(path);
        loadFile = new QFile(path);
        //打开文件，只读方式
        bool isOK = file.open(QIODevice::ReadOnly);
        if (isOK == true)
        {
            QByteArray array;
            while (file.atEnd() == false)
            {
                drawLineIndex++;
                //每次读一行
                array = file.readLine();
                fileParser(array);
            }
            if (step == 1)
            {
                ui->yourTurnlabel->hide();
                timeStart.start(1000);
            }
            update();
            ui->yourTurnlabel->hide();
        }
        else if (isOK == false) {
            int ret = QMessageBox::warning(this, "Error", "Please choose another readable file as the draw file!", QMessageBox::Ok);
            switch (ret)
            {
            case QMessageBox::Ok:
                break;
            default:
                break;
            }
        }
        //关闭文件
        file.close();
    }
}
