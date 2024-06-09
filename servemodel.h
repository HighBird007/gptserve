#ifndef SERVEMODEL_H
#define SERVEMODEL_H

#include <QTcpServer>
#include <tcpsocket.h>
#include <QThreadPool>
#include <showmain.h>
class servemodel : public QTcpServer
{
    Q_OBJECT
public:
    explicit servemodel(QObject *parent = nullptr);
protected:
    void incomingConnection(qintptr socketDescriptor) override;
private:

   showmain *smain;
public:
    ~servemodel();
};

#endif // SERVEMODEL_H
