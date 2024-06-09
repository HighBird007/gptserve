#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    serve=new QTcpServer;
    serve->listen(QHostAddress::Any,1207);
    connect(serve,&QTcpServer::newConnection,this,&Widget::incomingConnection);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::incomingConnection(qintptr socketdescriptor)
{
    qDebug()<<"new";
    QThread *thread=new QThread();
    tcpsocket* s=new tcpsocket(serve->nextPendingConnection()->socketDescriptor());
    s->moveToThread(thread);
    QWidget::QObject:: connect(s,&tcpsocket::disconnect,s,[&](){

    });
    QWidget::QObject:: connect(thread,&QThread::finished,thread,[](){
        qDebug()<<"???";
    });
    thread->start();
}
