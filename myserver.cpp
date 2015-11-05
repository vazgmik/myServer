#include "myserver.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QtWidgets>
#include <QDataStream>
#include <QtSql>

MyServer::MyServer(int nPort, QWidget *parent/*=0*/):
    QWidget(parent),m_nNextBlockSize(0)
{

    if(!createConnection())
        return;

    m_ptcpServer = new QTcpServer(this);
    if(!m_ptcpServer->listen(QHostAddress::Any,nPort)){
        QMessageBox::critical(0,"Server Error","Unable to start server:"+m_ptcpServer->errorString());
        m_ptcpServer->close();
        return;
    }
    connect(m_ptcpServer,SIGNAL(newConnection()),SLOT(slotNewConnection()));

    m_ptxt = new QTextEdit;
    m_ptxt->setReadOnly(true);


    QVBoxLayout * pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel("<h1>Server</h1>"));
    pvbxLayout->addWidget(m_ptxt);
    setLayout(pvbxLayout);

}

void MyServer::slotNewConnection()
{
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();
    connect(pClientSocket,SIGNAL(disconnected()),pClientSocket,SLOT(deleteLater()));
    connect(pClientSocket,SIGNAL(readyRead()),SLOT(slotReadClient()));
    sendToClient(pClientSocket,"Server Response:Connected!");
}

void MyServer::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_5_5);
    for(;;){
        if(!m_nNextBlockSize){
            if(pClientSocket->bytesAvailable() <(qint64)sizeof(quint64))
                break;
        }
        in >> m_nNextBlockSize;

        if(pClientSocket->bytesAvailable() <(qint64)m_nNextBlockSize){
            break;
        }

        QString str;
        in >> str;

        m_ptxt->append("Client has sent "+str);
        m_nNextBlockSize = 0;
        sendToClient(pClientSocket,str);
    }

}

void MyServer::sendToClient(QTcpSocket *pSocket,const QString &str)
{
    bool isNum;
    int num = str.toInt(&isNum);

    if (isNum)
    {
        QImage img(paths.at(num-1));
        QByteArray arrBlock;
        QDataStream out(&arrBlock,QIODevice::WriteOnly);
        QString typemsg = "image";
        out.setVersion(QDataStream::Qt_5_5);
        out << quint64(0) << typemsg << QTime::currentTime() << img;
        out.device()->seek(0);
        quint64 size = arrBlock.size()-sizeof(quint64);
        out << quint64(size);
        if(pSocket->write(arrBlock) < (qint64)size)
            qWarning("transmit error");
    }
    else
    {
        QVector<QStringList> v_rows;
        QString typemsg = "data";
        //read data from database
        QSqlQuery query;
        if(!query.exec("Select * from lab2;")){
             qDebug() <<"Unable to execute query:";
             return;
        }

        QSqlRecord rec = query.record();
        QString s1,s2,s3,s4;
        QStringList lst;

        while(query.next())
        {
            s1 = query.value(rec.indexOf("author")).toString();
            s2 = query.value(rec.indexOf("descr")).toString();
            s3 = query.value(rec.indexOf("date")).toString();
            s4 = query.value(rec.indexOf("path")).toString();
            lst<< s1<<s2 <<s3 <<s4;
            paths.append(s4);
            v_rows.append(lst);
            lst.clear();
            qDebug() << s1 << ";\t" << s2 <<";\t"<<s3<<";\t" << s4;
        }


        QByteArray arrBlock;
        QDataStream out(&arrBlock,QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_5_5);
        out << quint64(0) << typemsg << QTime::currentTime() << str<<v_rows;
        out.device()->seek(0);
        quint64 size = arrBlock.size()-sizeof(quint64);
        out << quint64(size);
        if(pSocket->write(arrBlock) < (qint64)size)
            qWarning("transmit error");
    }
}

bool MyServer::createConnection()
{
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("photo_db");
    db.setUserName("postgres");
    db.setHostName("localhost");
    db.setPassword("11091993");
    if(!db.open())
    {
        qDebug()<<"Cannot open db" <<db.lastError();
        return false;
    }
    return true;
}

MyServer::~MyServer()
{

}
