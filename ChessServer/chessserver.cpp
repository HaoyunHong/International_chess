#include "chessserver.h"
#include "ui_chessserver.h"

ChessServer::ChessServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChessServer)
{
    ui->setupUi(this);

    this->setFixedSize(500, 700);

    this->setWindowTitle("Server");

    ui->yourTurnlabel->setStyleSheet("color:white;background-color:red;");

    ui->yourTurnlabel->hide();

    isStart = false;

    QIcon mainIcon(":/img/img/whiteQueen.png");
    this->setWindowIcon(mainIcon);

    //菜单栏
    mBar = menuBar();
    setMenuBar(mBar);

    //添加菜单
    menu = mBar->addMenu("Option");
    actServer = menu->addAction("Listen");

    menu2 = mBar->addMenu("Game");
    actInitial = menu2->addAction("initial game");
    actLoad = menu2->addAction("load game");
    actSave = menu2->addAction("save game");

    port = 888;

    //ip也是要可编辑的
    connect(actServer, &QAction::triggered,
        [=]()
    {
        tcpServerServer = nullptr;
        tcpServerSocket = nullptr;
        tcpServerServer = new QTcpServer(this);

        tcpServerServer->listen(QHostAddress::LocalHost, port);
        this->setWindowTitle("Server is listening");

        connect(tcpServerServer, &QTcpServer::newConnection,
            [=]()
        {
            menu->setEnabled(false);
            //actServer->setEnabled(false);
            qDebug() << "newConnection!";
            tcpServerSocket = tcpServerServer->nextPendingConnection();
            //qDebug()<<"newConnection222!";
            //获取对方的IP和端口
            QString ip = tcpServerSocket->peerAddress().toString();
            //qDebug()<<"newConnection333!";
            quint16 port = tcpServerSocket->peerPort();
            QString temp = QString("Connected to client [%1：%2]").arg(ip).arg(port);
            // ui->textEdit->setText(temp);
            this->setWindowTitle(temp);
            qDebug() << temp;

            menu2->setEnabled(true);


            connect(actInitial, &QAction::triggered,
                [=]()
            {
                actInitial->setEnabled(false);
                actLoad->setEnabled(false);
                timerStart.start(1000);
            });

            connect(actLoad,&QAction::triggered,
                    [=]
            {
                actLoad->setEnabled(false);
                qDebug()<<"actLoad!";

                if(!isLoad)
                {
                QString path = QFileDialog::getOpenFileName(this,
                    "[Server] Please choose a draw file", "../serverDrawSave/", "TXT(*.txt)");
                //只有当文件不为空时才进行操作
                if (path.isEmpty() == false)
                {
                    actInitial->setEnabled(false);
                    //文件操作
                    QFile file(path);
                    loadFile = new QFile(path);
                    //打开文件，只读方式
                    bool isOK = file.open(QIODevice::ReadOnly);
                    if (isOK == true)
                    {
                        QByteArray block;
                        QDataStream out(&block, QIODevice::WriteOnly);
                        out.setVersion(QDataStream::Qt_4_3);

                        QStringList pathList = path.split("/");
                        qDebug()<<"pathList[pathList.size()-1]"<<pathList[pathList.size()-1];
                        //先写一个0来给将要传出去的信息的大小数据占个位子
                        out << quint16(12345)<<pathList[pathList.size()-1];
                        tcpServerSocket->write(block);

                        QByteArray array;
                        while (file.atEnd() == false)
                        {
                            drawLineIndex++;
                            qDebug()<<"In init drawLineIndex = "<<drawLineIndex;
                            //每次读一行
                            array = file.readLine();
                            fileParser(array);
                        }
                        update();
                        ui->yourTurnlabel->hide();
                        judgeCamp();


                        if(step==-1)
                        {
                            timerStart.start(1000);
                        }

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

                isLoad = true;
            });

            if(!isLose)
            {
                connect(ui->giveUpButton,&QPushButton::clicked,
                        [=]()
                {
                    isLose  = true;
                    ui->lcdNumber->display(0);
                    timerCount.stop();
                    QByteArray block;
                    QDataStream out(&block, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_3);
                    //先写一个0来给将要传出去的信息的大小数据占个位子
                    out << quint16(7777);
                    tcpServerSocket->write(block);

                    int ret = QMessageBox::information(this, "Lose", "[You Give Up] You Lose!", QMessageBox::Ok);
                    switch (ret)
                    {
                    case QMessageBox::Ok:
                        choice();
                        break;
                    default:
                        break;
                    }
                });
            }

            connect(tcpServerSocket, &QTcpSocket::readyRead,
                [=]()
            {
                //如果接到了客户端的信息应该怎么办
                qDebug() << "Server readyRead!";
                quint16 nextBlockSize = 0;
                QDataStream in(tcpServerSocket);

                in.setVersion(QDataStream::Qt_4_3);
                forever{

                    if(tcpServerSocket==nullptr)
                    {
                        break;
                    }
                    int oX,oY,tX,tY;

                    in >> nextBlockSize;
                    qDebug() << "nextBlockSize: " << nextBlockSize;

                    if(nextBlockSize == 7878)
                    {
                        timerCount.stop();
                        int ret = QMessageBox::information(this, "Draw", "Reach a draw!", QMessageBox::Ok);
                        switch (ret)
                        {
                        case QMessageBox::Ok:
                            choice();
                            break;
                        default:
                            choice();
                            break;
                        }
                        break;
                    }
                    if(nextBlockSize == 8000)
                    {
                        QPoint kingT;
                        in>>kingT;
                        QPoint rookO;
                        in>>rookO;
                        QPoint rookT;
                        in>>rookT;
                        matrix[4][0]=0;
                        matrix[kingT.x()][kingT.y()]=-6;
                        matrix[rookO.x()][rookO.y()]=0;
                        matrix[rookT.x()][rookT.y()]=-2;
                        update();

                        step++;
                        qDebug() << "Before Client turn step = " << step;
                        timerCount.start(1000);

                        judgeCamp();
                        return;
                    }
                    if(nextBlockSize == 7777)
                    {
                        timerCount.stop();
                        int ret = QMessageBox::information(this, "Win", "[Opposite Give Up] You Win!", QMessageBox::Ok);
                        switch (ret)
                        {
                            case QMessageBox::Ok:
                                choice();
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    if(nextBlockSize == 6666)
                    {
                        timerCount.stop();
                        int ret = QMessageBox::information(this, "Win", "[Opposite Time Out] You Win!", QMessageBox::Ok);
                        switch (ret)
                        {
                        case QMessageBox::Ok:
                            choice();
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
                        if (tcpServerSocket->bytesAvailable() < sizeof(quint16))
                        {
                            break;//比都有的信息的大小都小了，就说明没有包了
                        }
                        //in >> nextBlockSize;//要不然我们就可以获得后一个包的完整大小
                    }

                    if (tcpServerSocket->bytesAvailable() < nextBlockSize)
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

                        quint16 pro=0;
                        in>>pro;
                        qDebug()<<"in server pro: "<<pro;
                        if(pro>0)
                        {
                            matrix[tX][tY] = pro;
                        }

                        qDebug()<<"Server receive point:";
                        qDebug()<<opposeOrigin;
                        qDebug()<<opposeTo;

                        qDebug()<<"In server step: "<<step;
                        step++;
                        actSave->setEnabled(true);
                        qDebug()<<"Before Server turn step = "<<step;
                        update();

                        timerCount.start(1000);

                        judgeCamp();

                        if(pro == 505)
                        {
                            timerCount.stop();
                            int ret = QMessageBox::information(this, "Lose", "[Be Checkmated] You Lose!", QMessageBox::Ok);
                            switch (ret)
                            {
                            case QMessageBox::Ok:
                                choice();
                                break;
                            default:
                                choice();
                                break;
                            }
                        }
                }


            });

        });

        //其实弹窗里点ok是不需要做什么操作的
        sCDlg = new serverConnectDialog(this);
        connect(sCDlg, &serverConnectDialog::canConnect,
            [=](QString ipp)
        {
            ip = ipp;
            qDebug() << "ip = " << ip;

        });

        //但是弹窗里点cancel就可以断开连接
        connect(sCDlg, &serverConnectDialog::cannotConnect,
            [=]()
        {
            qDebug() << "Server Cancel!";
            if (nullptr == tcpServerSocket)
            {
                int ret = QMessageBox::question(this, "question", "Are you sure to cancel the game you initiate?", QMessageBox::Yes | QMessageBox::No);
                switch (ret)
                {
                case QMessageBox::Yes:
                    tcpServerServer->close();
                    menu->setEnabled(true);
                    break;
                case QMessageBox::No:
                    break;
                default:
                    break;
                }

                return;
            }
            //主动和客户端端口断开连接
            tcpServerServer->close();
            tcpServerSocket->disconnectFromHost();
            tcpServerSocket->close();
            tcpServerSocket = nullptr;
            this->setWindowTitle("Server canceled the connection");
            qDebug() << "Server Disconnected!";
            menu->setEnabled(true);
            actInitial->setEnabled(true);
            actLoad->setEnabled(true);
            menu2->setEnabled(false);

        });

        sCDlg->exec();

    });

    startTime = 3;

    countTime = 60;
    connect(&timerStart,&QTimer::timeout,
            [=]()
    {

        ui->lcdNumber->display(startTime);
        if(startTime==0)
        {
            ui->yourTurnlabel->show();
            if(!isLoad)
            {
                timerCount.start(1000);
                timerStart.stop();
                initial();
                step++;
                QByteArray block;
                QDataStream out(&block, QIODevice::WriteOnly);
                out.setVersion(QDataStream::Qt_4_3);
                QString str;
                int oX = -1;
                out << quint16(0) << oX;
                out.device()->seek(0);
                out << quint16(block.size() - sizeof(quint16));
                tcpServerSocket->write(block);
                actInitial->setEnabled(false);
                actLoad->setEnabled(false);
                update();
            }
            else if(isLoad&&step==-1)
            {
                step++;
                qDebug()<<"Load begin!";
                timerCount.start(1000);
                timerStart.stop();
            }
        }
        startTime--;

    });

    connect(&timerCount,&QTimer::timeout,
            [=]()
    {
        isStart = true;
        ui->lcdNumber->display(countTime);

        if(countTime==0)
        {
            timerCount.stop();
            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_3);
            //先写一个0来给将要传出去的信息的大小数据占个位子
            out << quint16(6666);
            tcpServerSocket->write(block);

            int ret = QMessageBox::information(this, "Lose", "[Time Out] You Lose!", QMessageBox::Ok);
            switch (ret)
            {
            case QMessageBox::Ok:
                choice();
                break;
            default:
                break;
            }
        }
        countTime--;

    });

    connect(actSave,&QAction::triggered,
            [=]()
    {
        int ret = QMessageBox::question(this, "Save", "Do you want to quit the game and save the draw?", QMessageBox::Yes|QMessageBox::No);
        switch (ret)
        {
        case QMessageBox::Yes:
            saveDraw();
            choice();
            break;
       case QMessageBox::No:
            choice();
            break;
        default:
            choice();
            break;
        }

    });

    focus = QPoint(-1, -1);
    curLeftClick = QPoint(-1, -1);
    menu2->setEnabled(false);
    isSelected = false;

    step = -1;

    drawLineIndex = 0;
    isLoad = false;

    for(int i=0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            matrix[i][j]=0;
        }
    }

    actSave->setEnabled(false);

    isLose = false;
}

ChessServer::~ChessServer()
{
    delete ui;
}

void ChessServer::paintEvent(QPaintEvent *e)
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

    if (step % 2 == 0 && step>=0)
    {
        if(isStart)
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
            p.drawRect(focus.x()*unit, focus.y()*unit, unit, unit);
            for (int i = 0; i < focusPath.size(); i++)
            {
                int x = focusPath[i].x();
                int y = focusPath[i].y();
                p.drawRect(x*unit, y*unit, unit, unit);
            }
        }
    }



    QPixmap wPawn(":/img/img/whitePawn.png");
    QPixmap bPawn(":/img/img/blackPawn.png");
    QPixmap wRook(":/img/img/whiteRook.png");
    QPixmap bRook(":/img/img/blackRook.png");
    QPixmap wHorse(":/img/img/whiteKnight.png");
    QPixmap bHorse(":/img/img/blackKnight.png");
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

void ChessServer::initial()
{
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            matrix[i][j] = 0;
        }
    }

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

void ChessServer::mousePressEvent(QMouseEvent *e)
{
    if(step%2 != 0 || !isStart)
    {
        return;
    }

    QPoint curPoint = e->pos();
    QPoint centerIJ;

    int unit = 50;

    QPoint o(50, 80);

    if (e->button() == Qt::LeftButton)
    {
        qDebug()<<"LeftButton";
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
                if (rec.contains(curPoint) && matrix[i][j] > 0)
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
                    if (focus == QPoint(4, 7) && matrix[focus.x()][focus.y()] == 6 && (QPoint(i, j) == QPoint(1, 7) || QPoint(i, j) == QPoint(2, 7) || QPoint(i, j) == QPoint(6, 7)))
                    {
                        matrix[i][j] = matrix[focus.x()][focus.y()];
                        matrix[focus.x()][focus.y()] = 0;

                        QPoint kingT = QPoint(i, j);
                        QPoint rookO;
                        QPoint rookT;
                        if (QPoint(i, j) == QPoint(1, 7))
                        {
                            matrix[0][7] = 0;
                            matrix[2][7] = 2;
                            rookO = QPoint(0, 7);
                            rookT = QPoint(2, 7);
                        }
                        if (QPoint(i, j) == QPoint(2, 7))
                        {
                            matrix[0][7] = 0;
                            matrix[3][7] = 2;
                            rookO = QPoint(0, 7);
                            rookT = QPoint(3, 7);
                        }
                        if (QPoint(i, j) == QPoint(6, 7))
                        {
                            matrix[7][7] = 0;
                            matrix[5][7] = 2;
                            rookO = QPoint(7, 7);
                            rookT = QPoint(5, 7);
                        }

                        update();

                        QByteArray block;
                        QDataStream out(&block, QIODevice::WriteOnly);
                        out.setVersion(QDataStream::Qt_4_3);
                        //先写一个0来给将要传出去的信息的大小数据占个位子
                        out << quint16(8000) << QPoint(i, j) << rookO << rookT;
                        tcpServerSocket->write(block);

                        timerCount.stop();
                        ui->lcdNumber->display(60);
                        countTime = 60;

                        focusPath.clear();
                        focus = QPoint(-1, -1);
                        step++;
                        qDebug()<<"This Server turn step = "<<step;
                        isSelected = false;
                        ui->yourTurnlabel->hide();

                        return;
                    }
                    actSave->setEnabled(true);

                    bool isEating = false;

                    if(matrix[i][j]==-6)
                    {
                        isEating=true;
                    }

                    matrix[i][j] = matrix[focus.x()][focus.y()];
                    matrix[focus.x()][focus.y()] = 0;

                    update();

                    if(isEating)
                    {
                        QByteArray block;
                        QDataStream out(&block, QIODevice::WriteOnly);
                        out.setVersion(QDataStream::Qt_4_3);
                        //先写一个0来给将要传出去的信息的大小数据占个位子
                        out << quint16(0) << focus.x() << focus.y() << i << j<<quint16(505);
                        //找到那个0，再覆盖掉
                        out.device()->seek(0);
                        out << quint16(block.size() - sizeof(quint16));
                        tcpServerSocket->write(block);

                        qDebug()<<"Server send:";
                        qDebug()<<quint16(block.size() - sizeof(quint16));
                        qDebug()<<focus;
                        qDebug()<<i<<","<<j;

                        int ret = QMessageBox::information(this, "Win", "[Checkmate] You Win!", QMessageBox::Ok);
                        switch (ret)
                        {
                        case QMessageBox::Ok:
                            choice();
                            break;
                        default:
                            choice();
                            break;
                        }

                        return;
                    }

                    //qDebug()<<"j = "<<j;
                    if(j==0 && matrix[i][j]==1)
                    {
                        //qDebug()<<"j = "<<j;
                        pdlg = new pawnProDialog(this);

                        connect(pdlg,&pawnProDialog::toQueen,
                                [=]()
                        {
                            matrix[i][j]=5;
                            qDebug()<<"toQueen";

                        });
                        connect(pdlg,&pawnProDialog::toBishop,
                                [=]()
                        {
                            matrix[i][j]=4;
                        });
                        connect(pdlg,&pawnProDialog::toHorse,
                                [=]()
                        {
                            matrix[i][j]=3;
                        });
                        connect(pdlg,&pawnProDialog::toRook,
                                [=]()
                        {
                            matrix[i][j]=2;
                        });

                        pdlg->exec();

                        update();

                        QByteArray block;
                        QDataStream out(&block, QIODevice::WriteOnly);
                        out.setVersion(QDataStream::Qt_4_3);
                        //先写一个0来给将要传出去的信息的大小数据占个位子
                        out << quint16(0) << focus.x() << focus.y() << i << j<<quint16(matrix[i][j]);
                        //找到那个0，再覆盖掉
                        out.device()->seek(0);
                        out << quint16(block.size() - sizeof(quint16));
                        tcpServerSocket->write(block);

                        qDebug()<<"Server send:";
                        qDebug()<<quint16(block.size() - sizeof(quint16));
                        qDebug()<<focus;
                        qDebug()<<i<<","<<j;
                    }
                    else {
                        QByteArray block;
                        QDataStream out(&block, QIODevice::WriteOnly);
                        out.setVersion(QDataStream::Qt_4_3);
                        //先写一个0来给将要传出去的信息的大小数据占个位子
                        out << quint16(0) << focus.x() << focus.y() << i << j<<quint16(0);
                        //找到那个0，再覆盖掉
                        out.device()->seek(0);
                        out << quint16(block.size() - sizeof(quint16));
                        tcpServerSocket->write(block);

                        qDebug()<<"Server send:";
                        qDebug()<<quint16(block.size() - sizeof(quint16));
                        qDebug()<<focus;
                        qDebug()<<i<<","<<j;
                    }

                    timerCount.stop();
                    ui->lcdNumber->display(60);
                    countTime = 60;

                    focusPath.clear();
                    focus = QPoint(-1, -1);
                    step++;
                    qDebug()<<"This Server turn step = "<<step;
                    isSelected = false;
                    ui->yourTurnlabel->hide();
                }
            }
        }
    }
}

void ChessServer::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(step%2!=0  || !isStart)
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
                if (rec.contains(curPoint) && matrix[i][j] > 0)
                {
                    isSelected = true;

                    focus = QPoint(i,j);
                    setMovePoints(focus);
                    if(focusPath.isEmpty())
                    {
                        isSelected = false;
                        focus = QPoint(-1,-1);
                        focusPath.clear();
                        qDebug()<<"can't move!";
                    }
                    else {
                        //就是表示这个追踪我不用了
                        curLeftClick = QPoint(-1,-1);
                        qDebug()<<"In Server: focus = "<<focus;
                    }

                    update();
                }
            }
        }

    }
}


QPixmap ChessServer::getPic(QPoint p)
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

void ChessServer::setMovePoints(QPoint curClick)
{
    int x = curClick.x();
    int y = curClick.y();

    //这里要注意的是兵升变的时候不能不变
    if (matrix[x][y] == 1)
    {
        qDebug() << "wPawn!";
        QPoint p(x, y - 1);
        if (matrix[x][y - 1] == 0)
        {
            curClickPath.push_back(p);
            if (y == 6)
            {
                curClickPath.push_back(QPoint(x, 4));
            }
        }
        if (x > 0 && y > 0 && matrix[x - 1][y - 1] < 0)
        {
            curClickPath.push_back(QPoint(x - 1, y - 1));
        }
        if (x < 7 && y>0 && matrix[x + 1][y - 1] < 0)
        {
            curClickPath.push_back(QPoint(x + 1, y - 1));
        }

        //        for(int i=0;i<curClickPath.size();i++)
        //        {
        //            if(matrix[curClickPath[i].x()][curClickPath[i].y()]!=0)
        //            {
        //                curClickPath.remove(i);
        //            }
        //        }
    }

    if (matrix[x][y] == 2)
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
                if (matrix[i][y] < 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                    break;
                }
                if (matrix[i][y] > 0)
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
                if (matrix[i][y] < 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                    break;
                }
                if (matrix[i][y] > 0)
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
                if (matrix[x][j] > 0)
                {
                    break;
                }
                if (matrix[x][j] < 0)
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
                if (matrix[x][j] > 0)
                {
                    break;
                }
                if (matrix[x][j] < 0)
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

    if (matrix[x][y] == 3)
    {
        qDebug() << "wHorse!";
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if ((i - x)*(i - x) + (j - y)*(j - y) == 5 && matrix[i][j] <= 0)
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

    if (matrix[x][y] == 4)
    {
        qDebug() << "wBishop!";
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
            else if (matrix[_x][_y] > 0)
            {
                break;
            }
            else if (matrix[_x][_y] < 0)
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
            else if (matrix[x_][y_] > 0)
            {
                break;
            }
            else if (matrix[x_][y_] < 0)
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
            else if (matrix[__x][__y] > 0)
            {
                break;
            }
            else if (matrix[__x][__y] < 0)
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
            else if (matrix[x__][y__] > 0)
            {
                break;
            }
            else if (matrix[x__][y__] < 0)
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

    if (matrix[x][y] == 5)
    {
        qDebug() << "wQueen!";
        if (x < 7)
        {
            for (int i = x + 1; i < 8; i++)
            {
                if (matrix[i][y] == 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                }
                if (matrix[i][y] < 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                    break;
                }
                if (matrix[i][y] > 0)
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
                if (matrix[i][y] < 0)
                {
                    curClickPath.push_back(QPoint(i, y));
                    break;
                }
                if (matrix[i][y] > 0)
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
                if (matrix[x][j] > 0)
                {
                    break;
                }
                if (matrix[x][j] < 0)
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
                if (matrix[x][j] > 0)
                {
                    break;
                }
                if (matrix[x][j] < 0)
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
            else if (matrix[_x][_y] > 0)
            {
                break;
            }
            else if (matrix[_x][_y] < 0)
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
            else if (matrix[x_][y_] > 0)
            {
                break;
            }
            else if (matrix[x_][y_] < 0)
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
            else if (matrix[__x][__y] > 0)
            {
                break;
            }
            else if (matrix[__x][__y] < 0)
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
            else if (matrix[x__][y__] > 0)
            {
                break;
            }
            else if (matrix[x__][y__] < 0)
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

    if (matrix[x][y] == 6)
    {
        qDebug() << "wKing!";
        if(QPoint(x,y)==QPoint(4,7))
        {
            dangerPoints.clear();
            QVector<QPoint> otherSide;
            for(int i=0;i<8;i++)
            {
                for(int j=0;j<8;j++)
                {
                     if(matrix[i][j]<0)
                     {
                          qDebug()<<"otherSide: "<<QPoint(i,j);
                          otherSide.push_back(QPoint(i,j));
                      }
                }
            }
            for(int i=0;i<otherSide.size();i++)
            {
                getPath(otherSide[i]);
            }


            if(matrix[0][7]==2 && matrix[1][7]==0 && matrix[2][7]==0 && matrix[3][7]==0)
            {
                int cnt=0;
                for(int i=0;i<dangerPoints.size();i++)
                {
                    qDebug()<<"dangerPoints["<<i<<"]: "<<dangerPoints[i];
                    if(dangerPoints[i]!=QPoint(4,7)  && dangerPoints[i]!=QPoint(2,7) && dangerPoints[i]!=QPoint(3,7))
                    {
                        cnt++;
                    }
                }
                if(cnt==dangerPoints.size())
                {
                    curClickPath.push_back(QPoint(2,7));
                }
            }

            if(matrix[7][7]==2 && matrix[6][7]==0 && matrix[5][7]==0)
            {
                int cnt=0;
                for(int i=0;i<dangerPoints.size();i++)
                {
                    if(dangerPoints[i]!=QPoint(4,7)  && dangerPoints[i]!=QPoint(5,7) && dangerPoints[i]!=QPoint(6,7))
                    {
                        cnt++;
                    }
                }
                if(cnt==dangerPoints.size())
                {
                    curClickPath.push_back(QPoint(6,7));
                }
            }


        }
        if (x > 0 && matrix[x - 1][y] <= 0)
        {
            curClickPath.push_back(QPoint(x - 1, y));
        }
        if (x > 0 && y > 0 && matrix[x - 1][y - 1] <= 0)
        {
            curClickPath.push_back(QPoint(x - 1, y - 1));
        }
        if (x > 0 && y < 7 && matrix[x - 1][y + 1] <= 0)
        {
            curClickPath.push_back(QPoint(x - 1, y + 1));
        }
        if (y > 0 && matrix[x][y - 1] <= 0)
        {
            curClickPath.push_back(QPoint(x, y - 1));
        }
        if (y < 7 && matrix[x][y + 1] <= 0)
        {
            curClickPath.push_back(QPoint(x, y + 1));
        }
        if (x < 7 && y>0 && matrix[x + 1][y - 1] <= 0)
        {
            curClickPath.push_back(QPoint(x + 1, y - 1));
        }
        if (x < 7 && matrix[x + 1][y] <= 0)
        {
            curClickPath.push_back(QPoint(x + 1, y));
        }
        if (x < 7 && y < 7 && matrix[x + 1][y + 1] <= 0)
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

void ChessServer::choice()
{
    int ret = QMessageBox::question(this, "question", "Do you want to play again? If so, click Yes. Otherwise you will be disconnected from the current opponent.", QMessageBox::Yes | QMessageBox::No);
    switch (ret)
    {
    case QMessageBox::Yes:
        playAgain();
        break;
    case QMessageBox::No:
//        tcpServerServer->close();
//        tcpServerSocket->disconnectFromHost();
//        tcpServerSocket->close();
//        tcpServerServer = nullptr;
//        tcpServerSocket = nullptr;
        origin();
        break;
    default:
        break;
    }
}

void ChessServer::playAgain()
{
    actSave->setEnabled(false);
    isLose = false;
    actSave->setEnabled(false);
    timerCount.stop();
    drawLineIndex=0;
    isStart = false;
    isLoad = false;
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_3);
    //先写一个0来给将要传出去的信息的大小数据占个位子
    out << quint16(8888);
    tcpServerSocket->write(block);

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
    step = -1;
    startTime = 3;
    countTime = 60;

    drawLineIndex = 0;
    isLoad = false;

    update();

    menu->setEnabled(false);
    actInitial->setEnabled(true);
    actLoad->setEnabled(true);
    actSave->setEnabled(true);
    ui->lcdNumber->display(0);
}


void ChessServer::origin()
{
    isLose = false;
    actSave->setEnabled(false);
    ui->lcdNumber->display(0);
    drawLineIndex = 0;
    isLoad = false;
    isStart = false;
    drawLineIndex = 0;

    this->setWindowTitle("Server");
    timerCount.stop();

    ui->yourTurnlabel->hide();

    if (nullptr == tcpServerSocket)
    {
        int ret = QMessageBox::question(this, "question", "Are you sure to cancel the game you initiate?", QMessageBox::Yes | QMessageBox::No);
        switch (ret)
        {
        case QMessageBox::Yes:
            tcpServerServer->close();
            tcpServerServer = nullptr;
            menu->setEnabled(true);
            break;
        case QMessageBox::No:
            break;
        default:
            break;
        }

        return;
    }
    //主动和客户端端口断开连接
    tcpServerServer->close();
    tcpServerSocket->disconnectFromHost();

    tcpServerSocket->close();

    qDebug()<<"In server tcpServerServer->disconnect(): "<<tcpServerServer->disconnect();;\
    tcpServerSocket = nullptr;
    tcpServerServer = nullptr;

    this->setWindowTitle("Server canceled the connection");
    qDebug() << "Server Disconnected!";
    menu->setEnabled(true);
    actInitial->setEnabled(true);
    actLoad->setEnabled(true);
    menu2->setEnabled(false);

    focus = QPoint(-1, -1);
    curLeftClick = QPoint(-1, -1);
    isSelected = false;
    step = -1;
    startTime = 3;
    countTime = 60;


    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            matrix[i][j] = 0;
        }
    }
    update();
    qDebug() << "Server Cancel!";

    actSave->setEnabled(false);
}



void ChessServer::fileParser(QByteArray array)
{
    QString str(array);
    qDebug()<<"In fileParser: "<<str;
    QStringList strList = str.split(" ");

    int num = 0;

    QPoint* chessPos;

    qDebug()<<"drawLineIndex = "<<drawLineIndex;

    if(drawLineIndex == 1)
    {
        if(strList[0].contains("white"))
        {
            qDebug()<<"white first!";
            step = -1;
            isMine = true;
        }
        else if(strList[0].contains("black"))
        {
            qDebug()<<"black first!";
            step = 1;
            isMine = false;
        }

    }
    else {
        if(strList[0].contains("white"))
        {
            qDebug()<<"white second!";
            isMine = true;
        }
        else if(strList[0].contains("black"))
        {
            qDebug()<<"black second!";
            isMine = false;
        }

        if(strList.size()>2)
        {
            num = strList[1].toInt();
            chessPos = new QPoint[num];

            for(int i=2;i<num+2;i++)
            {
                if(strList[i][0]=="a")
                {
                    chessPos[i-2].setX(0);
                }
                if(strList[i][0]=="b")
                {
                    chessPos[i-2].setX(1);
                }
                if(strList[i][0]=="c")
                {
                    chessPos[i-2].setX(2);
                }
                if(strList[i][0]=="d")
                {
                    chessPos[i-2].setX(3);
                }
                if(strList[i][0]=="e")
                {
                    chessPos[i-2].setX(4);
                }
                if(strList[i][0]=="f")
                {
                    chessPos[i-2].setX(5);
                }
                if(strList[i][0]=="g")
                {
                    chessPos[i-2].setX(6);
                }
                if(strList[i][0]=="h")
                {
                    chessPos[i-2].setX(7);
                }
                if(strList[i][1]=="1")
                {
                    chessPos[i-2].setY(7);
                }
                if(strList[i][1]=="2")
                {
                    chessPos[i-2].setY(6);
                }
                if(strList[i][1]=="3")
                {
                    chessPos[i-2].setY(5);
                }
                if(strList[i][1]=="4")
                {
                    chessPos[i-2].setY(4);
                }
                if(strList[i][1]=="5")
                {
                    chessPos[i-2].setY(3);
                }
                if(strList[i][1]=="6")
                {
                    chessPos[i-2].setY(2);
                }
                if(strList[i][1]=="7")
                {
                    chessPos[i-2].setY(1);
                }
                if(strList[i][1]=="8")
                {
                    chessPos[i-2].setY(0);
                }
            }

            qDebug()<<"isMine: "<<isMine;
            if(strList[0]=="king")
            {
                for(int i=0;i<num;i++)
                {
                    if(isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()]=6;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()]=-6;
                    }
                }
            }
            if(strList[0]=="queen")
            {
                for(int i=0;i<num;i++)
                {
                    if(isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()]=5;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()]=-5;
                    }
                }
            }
            if(strList[0]=="bishop")
            {
                for(int i=0;i<num;i++)
                {
                    if(isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()]=4;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()]=-4;
                    }
                }
            }
            if(strList[0]=="knight")
            {
                for(int i=0;i<num;i++)
                {
                    if(isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()]=3;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()]=-3;
                    }
                }
            }
            if(strList[0]=="rook")
            {
                for(int i=0;i<num;i++)
                {
                    if(isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()]=2;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()]=-2;
                    }
                }
            }
            if(strList[0]=="pawn")
            {
                for(int i=0;i<num;i++)
                {
                    if(isMine)
                    {
                        matrix[chessPos[i].x()][chessPos[i].y()]=1;
                    }
                    else {
                        matrix[chessPos[i].x()][chessPos[i].y()]=-1;
                    }
                }
            }
        }


        for(int j=0;j<num;j++)
        {
            qDebug()<<drawLineIndex<<": chessPos["<<j<<"]"<<chessPos[j];
        }
    }

}

void ChessServer::saveFill()
{
    whiteStore[1]<<"pawn";
    whiteStore[2]<<"rook";
    whiteStore[3]<<"knight";
    whiteStore[4]<<"bishop";
    whiteStore[5]<<"queen";
    whiteStore[6]<<"king";

    blackStore[1]<<"pawn";
    blackStore[2]<<"rook";
    blackStore[3]<<"knight";
    blackStore[4]<<"bishop";
    blackStore[5]<<"queen";
    blackStore[6]<<"king";

    for(int i=0;i<8;i++)
    {
        for(int j=0;j<8;j++)
        {
            QString al;
            QString num;
            if(matrix[i][j] != 0)
            {
                if(i==0)
                {
                    al="a";
                }
                if(i==1)
                {
                    al="b";
                }
                if(i==2)
                {
                    al="c";
                }
                if(i==3)
                {
                    al="d";
                }
                if(i==4)
                {
                    al="e";
                }
                if(i==5)
                {
                    al="f";
                }
                if(i==6)
                {
                    al="g";
                }
                if(i==7)
                {
                    al="h";
                }

                num = QString::number(8-j);
            }

            if(matrix[i][j]==1)
            {
                whiteStore[1]<<al+num;
            }
            if(matrix[i][j]==2)
            {
                whiteStore[2]<<al+num;
            }
            if(matrix[i][j]==3)
            {
                whiteStore[3]<<al+num;
            }
            if(matrix[i][j]==4)
            {
                whiteStore[4]<<al+num;
            }
            if(matrix[i][j]==5)
            {
                whiteStore[5]<<al+num;
            }
            if(matrix[i][j]==6)
            {
                whiteStore[6]<<al+num;
            }

            if(matrix[i][j]==-1)
            {
                blackStore[1]<<al+num;
            }
            if(matrix[i][j]==-2)
            {
                blackStore[2]<<al+num;
            }
            if(matrix[i][j]==-3)
            {
                blackStore[3]<<al+num;
            }
            if(matrix[i][j]==-4)
            {
                blackStore[4]<<al+num;
            }
            if(matrix[i][j]==-5)
            {
                blackStore[5]<<al+num;
            }
            if(matrix[i][j]==-6)
            {
                blackStore[6]<<al+num;
            }

        }
    }

}


void ChessServer::saveDraw()
{
    timerCount.stop();

    QString path = QFileDialog::getSaveFileName(this,
                                                "[Server] Please save the draw file!", "../serverDrawSave/", "TXT(*.txt)");
    if (path.isEmpty() == false)
    {

        QFile file;//创建文件对象
        //关联文件名字
        file.setFileName(path);

        blackStore = new QStringList[7];
        whiteStore = new QStringList[7];
        saveFill();

        QTextStream txtin(&file);

        //打开文件，只写方式
        bool isOK = file.open(QIODevice::WriteOnly|QIODevice::Text);
        if (isOK == true)
        {

            QStringList pathList = path.split("/");
            qDebug()<<"pathList[pathList.size()-1]"<<pathList[pathList.size()-1];

            QByteArray block;
            QDataStream out(&block, QIODevice::WriteOnly);
            out.setVersion(QDataStream::Qt_4_3);
            out << quint16(1111) << pathList[pathList.size()-1];
            tcpServerSocket->write(block);

            if(step%2==0)
            {
                txtin<<"white\n";
                for(int i=1;i<7;i++)
                {
                    if(whiteStore[i].size()>1)
                    {
                        txtin<<whiteStore[i][0]+" ";
                        txtin<<QString::number(whiteStore[i].size()-1)+" ";
                        for(int j=1;j<whiteStore[i].size()-1;j++)
                        {
                            txtin<<whiteStore[i][j]+" ";
                        }
                        txtin<<whiteStore[i][whiteStore[i].size()-1]+"\n";
                    }
                }

                txtin<<"black\n";
                for(int i=1;i<7;i++)
                {
                    if(blackStore[i].size()>1)
                    {
                        txtin<<blackStore[i][0]+" ";
                        txtin<<QString::number(blackStore[i].size()-1)+" ";
                        for(int j=1;j<blackStore[i].size()-1;j++)
                        {
                            txtin<<blackStore[i][j]+" ";
                        }
                        txtin<<blackStore[i][blackStore[i].size()-1]+"\n";
                    }
                }

            }
            else {
                txtin<<"black\n";
                for(int i=1;i<7;i++)
                {
                    if(blackStore[i].size()>1)
                    {
                        txtin<<blackStore[i][0]+" ";
                        txtin<<QString::number(blackStore[i].size()-1)+" ";
                        for(int j=1;j<blackStore[i].size()-1;j++)
                        {
                            txtin<<blackStore[i][j]+" ";
                        }
                        txtin<<blackStore[i][blackStore[i].size()-1]+"\n";
                    }
                }
                txtin<<"white\n";
                for(int i=1;i<7;i++)
                {
                    if(whiteStore[i].size()>1)
                    {
                        txtin<<whiteStore[i][0]+" ";
                        txtin<<QString::number(whiteStore[i].size()-1)+" ";
                        for(int j=1;j<whiteStore[i].size()-1;j++)
                        {
                            txtin<<whiteStore[i][j]+" ";
                        }
                        txtin<<whiteStore[i][whiteStore[i].size()-1]+"\n";
                    }
                }
            }
        }
        file.close();
    }
}

void ChessServer::getPath(QPoint p)
{
    int x = p.x();
    int y = p.y();
    if (matrix[x][y] == -1)
    {
        qDebug() << "bPawn!";
        QPoint p(x, y + 1);
        if (matrix[x][y + 1] == 0)
        {
            dangerPoints.push_back(p);
            if (y == 1)
            {
                dangerPoints.push_back(QPoint(x, 3));
            }
        }
        if (x > 0 && y < 7 && matrix[x - 1][y + 1]>0)
        {
            dangerPoints.push_back(QPoint(x - 1, y + 1));
        }
        if (x < 7 && y < 7 && matrix[x + 1][y + 1]>0)
        {
            dangerPoints.push_back(QPoint(x + 1, y + 1));
        }
    }

    if (matrix[x][y] == -2)
    {
        qDebug() << "bRook!";
        if (x < 7)
        {
            for (int i = x + 1; i < 8; i++)
            {
                if (matrix[i][y] == 0)
                {
                    dangerPoints.push_back(QPoint(i, y));
                }
                if (matrix[i][y] > 0)
                {
                    dangerPoints.push_back(QPoint(i, y));
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
                    dangerPoints.push_back(QPoint(i, y));
                }
                if (matrix[i][y] > 0)
                {
                    dangerPoints.push_back(QPoint(i, y));
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
                    dangerPoints.push_back(QPoint(x, j));
                }
                if (matrix[x][j] < 0)
                {
                    break;
                }
                if (matrix[x][j] > 0)
                {
                    dangerPoints.push_back(QPoint(x, j));
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
                    dangerPoints.push_back(QPoint(x, j));
                }
                if (matrix[x][j] < 0)
                {
                    break;
                }
                if (matrix[x][j] > 0)
                {
                    dangerPoints.push_back(QPoint(x, j));
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
        qDebug() << "bKnight!";
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if ((i - x)*(i - x) + (j - y)*(j - y) == 5 && matrix[i][j] >= 0)
                {
                    dangerPoints.push_back(QPoint(i, j));
                }
            }
        }
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
                dangerPoints.push_back(QPoint(_x, _y));
            }
            else if (matrix[_x][_y] < 0)
            {
                break;
            }
            else if (matrix[_x][_y] > 0)
            {
                dangerPoints.push_back(QPoint(_x, _y));
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
                dangerPoints.push_back(QPoint(x_, y_));
            }
            else if (matrix[x_][y_] < 0)
            {
                break;
            }
            else if (matrix[x_][y_] > 0)
            {
                dangerPoints.push_back(QPoint(x_, y_));
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
                dangerPoints.push_back(QPoint(__x, __y));
            }
            else if (matrix[__x][__y] < 0)
            {
                break;
            }
            else if (matrix[__x][__y] > 0)
            {
                dangerPoints.push_back(QPoint(__x, __y));
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
                dangerPoints.push_back(QPoint(x__, y__));
            }
            else if (matrix[x__][y__] < 0)
            {
                break;
            }
            else if (matrix[x__][y__] > 0)
            {
                dangerPoints.push_back(QPoint(x__, y__));
                break;
            }
        }
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
                    dangerPoints.push_back(QPoint(i, y));
                }
                if (matrix[i][y] > 0)
                {
                    dangerPoints.push_back(QPoint(i, y));
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
                    dangerPoints.push_back(QPoint(i, y));
                }
                if (matrix[i][y] > 0)
                {
                    dangerPoints.push_back(QPoint(i, y));
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
                    dangerPoints.push_back(QPoint(x, j));
                }
                if (matrix[x][j] < 0)
                {
                    break;
                }
                if (matrix[x][j] > 0)
                {
                    dangerPoints.push_back(QPoint(x, j));
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
                    dangerPoints.push_back(QPoint(x, j));
                }
                if (matrix[x][j] < 0)
                {
                    break;
                }
                if (matrix[x][j] > 0)
                {
                    dangerPoints.push_back(QPoint(x, j));
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
                dangerPoints.push_back(QPoint(_x, _y));
            }
            else if (matrix[_x][_y] < 0)
            {
                break;
            }
            else if (matrix[_x][_y] > 0)
            {
                dangerPoints.push_back(QPoint(_x, _y));
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
                dangerPoints.push_back(QPoint(x_, y_));
            }
            else if (matrix[x_][y_] < 0)
            {
                break;
            }
            else if (matrix[x_][y_] > 0)
            {
                dangerPoints.push_back(QPoint(x_, y_));
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
                dangerPoints.push_back(QPoint(__x, __y));
            }
            else if (matrix[__x][__y] < 0)
            {
                break;
            }
            else if (matrix[__x][__y] > 0)
            {
                dangerPoints.push_back(QPoint(__x, __y));
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
                dangerPoints.push_back(QPoint(x__, y__));
            }
            else if (matrix[x__][y__] < 0)
            {
                break;
            }
            else if (matrix[x__][y__] > 0)
            {
                dangerPoints.push_back(QPoint(x__, y__));
                break;
            }
        }
    }

    if (matrix[x][y] == -6)
    {
        qDebug() << "bKing!";
        if (x > 0 && matrix[x - 1][y] >= 0)
        {
            dangerPoints.push_back(QPoint(x - 1, y));
        }
        if (x > 0 && y > 0 && matrix[x - 1][y - 1] >= 0)
        {
            dangerPoints.push_back(QPoint(x - 1, y - 1));
        }
        if (x > 0 && y < 7 && matrix[x - 1][y + 1] >= 0)
        {
            dangerPoints.push_back(QPoint(x - 1, y + 1));
        }
        if (y > 0 && matrix[x][y - 1] >= 0)
        {
            dangerPoints.push_back(QPoint(x, y - 1));
        }
        if (y < 7 && matrix[x][y + 1] >= 0)
        {
            dangerPoints.push_back(QPoint(x, y + 1));
        }
        if (x < 7 && y>0 && matrix[x + 1][y - 1] >= 0)
        {
            dangerPoints.push_back(QPoint(x + 1, y - 1));
        }
        if (x < 7 && matrix[x + 1][y] >= 0)
        {
            dangerPoints.push_back(QPoint(x + 1, y));
        }
        if (x < 7 && y < 7 && matrix[x + 1][y + 1] >= 0)
        {
            dangerPoints.push_back(QPoint(x + 1, y + 1));
        }
    }
}


//其实这里写得还不够完善
//逼和的另一种情况是如果移动了自己的可以移动的棋子，会导致直接被将军
//这里其实还可以再完善一下
void ChessServer::judgeCamp()
{
    if(step%2==1)
    {
        return;
    }
    bool isDraw = false;
    dangerPoints.clear();
    QPoint myKing;
    QVector<QPoint> myKingPath;

    QVector<QPoint> threatPoints;
    QVector<QPoint> dangers;

    QVector<QPoint> otherSide;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (matrix[i][j] < 0)
            {
                otherSide.push_back(QPoint(i, j));
                qDebug()<<"otherSide: "<<QPoint(i, j);
            }
            if(matrix[i][j]==6)
            {
                myKing=QPoint(i,j);
                qDebug()<<"myKing: "<<myKing;
            }
        }
    }

    int x = myKing.x();
    int y = myKing.y();
    if (x > 0 && matrix[x - 1][y] <= 0)
    {
        myKingPath.push_back(QPoint(x - 1, y));
    }
    if (x > 0 && y > 0 && matrix[x - 1][y - 1] <= 0)
    {
        myKingPath.push_back(QPoint(x - 1, y - 1));
    }
    if (x > 0 && y < 7 && matrix[x - 1][y + 1] <= 0)
    {
        myKingPath.push_back(QPoint(x - 1, y + 1));
    }
    if (y > 0 && matrix[x][y - 1] <= 0)
    {
        myKingPath.push_back(QPoint(x, y - 1));
    }
    if (y < 7 && matrix[x][y + 1] <= 0)
    {
        myKingPath.push_back(QPoint(x, y + 1));
    }
    if (x < 7 && y>0 && matrix[x + 1][y - 1] <= 0)
    {
        myKingPath.push_back(QPoint(x + 1, y - 1));
    }
    if (x < 7 && matrix[x + 1][y] <= 0)
    {
        myKingPath.push_back(QPoint(x + 1, y));
    }
    if (x < 7 && y < 7 && matrix[x + 1][y + 1] <= 0)
    {
        myKingPath.push_back(QPoint(x + 1, y + 1));
    }

    for(int i=0;i<otherSide.size();i++)
    {
        getPath(otherSide[i]);
        //dangers用来储存一长串
        dangers+=dangerPoints;
        //如果正在被将军就不是逼和
        if(dangerPoints.contains(myKing))
        {
            return;
        }
        for(int j=0;j<myKingPath.size();j++)
        {
            if(dangerPoints.contains(myKingPath[j]))
            {
                //threatPoints是会造成威胁的点
                threatPoints.push_back(otherSide[i]);
                qDebug()<<"threatPoints: "<<otherSide[i];
            }
        }

    dangerPoints.clear();
    }

    QVector<QPoint> mySide;
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (matrix[i][j] > 0)
            {
                mySide.push_back(QPoint(i, j));
                qDebug()<<"mySide: "<<QPoint(i, j);
            }
        }
    }

    QVector<QPoint> myDefendPoints;
    for(int i=0;i<mySide.size();i++)
    {
        getPath(mySide[i]);
        myDefendPoints+=dangerPoints;
        dangerPoints.clear();
    }

    for(int i=0;i<threatPoints.size();i++)
    {
        //如果王的某个位置能被守护住的话，也不是逼和
        if(myDefendPoints.contains(threatPoints[i]))
        {
            QVector<QPoint> remainThreat;
            for(int j=0;j<threatPoints.size();j++)
            {
                if(j!=i)
                {

                    getPath(threatPoints[j]);
                    remainThreat+=dangerPoints;
                    dangerPoints.clear();
                }
            }
            for(int k=0;k<myKingPath.size();k++)
            {
                if(!remainThreat.contains(myKingPath[k]))
                {
                    return;
                }
            }

        }
    }


    int cnt=0;
    for(int i=0;i<myKingPath.size();i++)
    {
        if(dangers.contains(myKingPath[i]))
        {
            cnt++;
            qDebug()<<"cnt: "<<cnt;
        }
    }


    if(cnt>0 && cnt==myKingPath.size())
    {
        timerCount.stop();
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_3);
        out << quint16(7878);
        tcpServerSocket->write(block);

        int ret = QMessageBox::information(this, "Draw", "Reach a draw!", QMessageBox::Ok);
        switch (ret)
        {
        case QMessageBox::Ok:
            choice();
            break;
        default:
            choice();
            break;
        }

    }
    return;
}
