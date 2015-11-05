#ifndef MYSERVER_H
#define MYSERVER_H

#include <QWidget>
#include <QSqlDatabase>

class QTcpServer;
class QTextEdit;
class QTcpSocket;

class MyServer : public QWidget
{
    Q_OBJECT

private:
    QTcpServer *m_ptcpServer;
    QTextEdit* m_ptxt;
    quint64 m_nNextBlockSize;
    QSqlDatabase db;
    QStringList paths;
private:
    void sendToClient(QTcpSocket *pSocket,const QString &str);
    bool createConnection();

public:
    MyServer(int nPort,QWidget *parent = 0);
    ~MyServer();
signals:

public slots:
    virtual void slotNewConnection();
    void slotReadClient();
};
#endif // MYSERVER_H
