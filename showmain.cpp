#include "showmain.h"
#include "ui_showmain.h"

showmain::showmain(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::showmain)
{
    ui->setupUi(this);
    udp = new QUdpSocket(this);
}

showmain::~showmain()
{
    delete ui;
}

void showmain::addtext(QString text)
{
    ui->textEdit->append(text);
}

void showmain::on_pushButton_clicked()
{
    QByteArray data = ui->textEdit_2->toPlainText().toUtf8();
    udp->writeDatagram(data,QHostAddress::Broadcast,9998);
}

