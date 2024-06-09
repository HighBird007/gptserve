#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QThread>
#include <tcpsocket.h>
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget ,public QTcpServer
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
protected:
       void incomingConnection(qintptr socketdescriptor) override;
private:
    Ui::Widget *ui;
    QTcpServer *serve;
};
#endif // WIDGET_H
