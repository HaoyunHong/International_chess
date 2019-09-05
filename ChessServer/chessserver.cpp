#include "chessserver.h"
#include "ui_chessserver.h"

ChessServer::ChessServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChessServer)
{
    ui->setupUi(this);

    this->setFixedSize(500, 700);

    this->setWindowTitle("Server");

    //菜单栏
    QMenuBar *mBar = menuBar();
    setMenuBar(mBar);

    //添加菜单
    QMenu *menu = mBar->addMenu("Option");
    QAction *actServer = menu->addAction("Listen");

    QMenu *menu2 = mBar->addMenu("Game");
    QAction *actInitial = menu2->addAction("initial game");
    QAction *actLoad = menu2->addAction("load game");
    QAction *actSave = menu2->addAction("save game");

    port = 6666;

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            matrix[i][j] = 0;
        }
    }



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
                initial();
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
            });

            connect(tcpServerSocket, &QTcpSocket::readyRead,
                [=]()
            {
                //如果接到了客户端的信息应该怎么办
                qDebug() << "Server readyRead!";
                quint16 nextBlockSize = 0;
                QDataStream in(tcpServerSocket);

                in.setVersion(QDataStream::Qt_4_3);
                forever{
                    int oX,oY,tX,tY;

                    in >> nextBlockSize;
                    qDebug() << "nextBlockSize: " << nextBlockSize;
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

                        step++;
                        qDebug()<<"Before Server turn step = "<<step;
                        update();
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

    QIcon mainIcon(":/img/img/whiteQueen.png");
    this->setWindowIcon(mainIcon);

    focus = QPoint(-1, -1);
    curLeftClick = QPoint(-1, -1);
    menu2->setEnabled(false);
    isSelected = false;
    hasDestination = false;

    step = 0;

     ui->yourTurnlabel->hide();
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

    if (step % 2 == 0)
    {
         ui->yourTurnlabel->show();
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
            p.drawRect(curLeftClick.x()*unit, curLeftClick.y()*unit, unit, unit);

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


    //棋盘中的方格的位子是要反一反的
//    if(isInitial)
//    {
//        //isInitial = false;
//        for(int i=0;i<8;i++)
//        {
//            for(int j=0;j<8;j++)
//            {
//                if(j==6)
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,wPawn);
//                }
//                if(j==1)
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,bPawn);
//                }
//                if(j==7 && (i==0||i==7))
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,wRook);
//                }
//                if(j==0 && (i==0||i==7))
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,bRook);
//                }
//                if(j==7 && (i==1||i==6))
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,wHorse);
//                }
//                if(j==0 && (i==1||i==6))
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,bHorse);
//                }
//                if(j==7 && (i==2||i==5))
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,wBishop);
//                }
//                if(j==0 && (i==2||i==5))
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,bBishop);
//                }
//                if(j==7 && (i==3))
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,wQueen);
//                }
//                if(j==0 && (i==3))
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,bQueen);
//                }
//                if(j==7 && (i==4))
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,wKing);
//                }
//                if(j==0 && (i==4))
//                {
//                    p.drawPixmap(i*unit,j*unit,unit,unit,bKing);
//                }
//            }
//        }

//    }


//    if(hasDestination)
//    {
//        p.drawPixmap(*unit,j*unit,unit,unit,bKing);
//        focus = QPoint(-1,-1);
//        focusPath.clear();
//    }



    p.end();
}

void ChessServer::initial()
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

void ChessServer::mousePressEvent(QMouseEvent *e)
{

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
                { //
                    qDebug() << "Here!";
                    hasDestination = true;

                    matrix[i][j] = matrix[focus.x()][focus.y()];
                    matrix[focus.x()][focus.y()] = 0;

                    QByteArray block;
                    QDataStream out(&block, QIODevice::WriteOnly);
                    out.setVersion(QDataStream::Qt_4_3);
                    //先写一个0来给将要传出去的信息的大小数据占个位子
                    out << quint16(0) << focus.x() << focus.y() << i << j;
                    //找到那个0，再覆盖掉
                    out.device()->seek(0);
                    out << quint16(block.size() - sizeof(quint16));
                    tcpServerSocket->write(block);


                    update();
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

                    setMovePoints(QPoint(i, j));

                    focus = QPoint(i, j);

                    //就是表示这个追踪我不用了
                    curLeftClick = QPoint(-1, -1);

                    setMovePoints(focus);

                    if(focusPath.size()>0)
                    {
                        qDebug() << "In Server: focus = " << focus;

                        update();
                    }
                    else {
                        qDebug() << "Cannot move!" << focus;
                        focus = QPoint(-1, -1);
                        focusPath.clear();

                    }



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

