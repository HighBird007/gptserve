#include "servemodel.h"

servemodel::servemodel(QObject *parent)
    : QTcpServer{parent}
{
    qDebug()<<"yes";
    smain=new showmain();

     smain->show();
    if (this->listen(QHostAddress::Any,52310)) {
        qDebug() << "Server is listening on address: " <<this->serverAddress();
        qDebug() << "Server start at port: " << this->serverPort();
        qDebug()<<this->serverAddress();
       smain->addtext(QString::number(this->serverPort())+"111");
    } else {
        smain->addtext( "Start failure");
        qDebug()<<this->errorString();
    }

}

void servemodel::incomingConnection(qintptr socketDescriptor)
{

        qDebug()<<"new";
        QThread *thread =new QThread();
        tcpsocket *socket=new tcpsocket(socketDescriptor);
         smain->addtext("new");
        socket->moveToThread(thread);
        connect(socket,&tcpsocket::disconnectsocket,thread,[=](){
            thread->quit();
            thread->wait();
            thread->deleteLater();
        });
         connect(socket,&tcpsocket::whathappen,smain,&showmain::addtext);
        connect(thread,&QThread::started,socket,&tcpsocket::runwork);
        thread->start();

}


servemodel::~servemodel()
{
    qDebug()<<"end";
}
