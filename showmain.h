#ifndef SHOWMAIN_H
#define SHOWMAIN_H

#include <QWidget>
#include <QUdpSocket>

namespace Ui {
class showmain;
}

class showmain : public QWidget
{
    Q_OBJECT

public:
    explicit showmain(QWidget *parent = nullptr);
    ~showmain();
    void addtext(QString text);
private slots:
    void on_pushButton_clicked();

private:
    Ui::showmain *ui;
    QUdpSocket *udp;
};

#endif // SHOWMAIN_H
