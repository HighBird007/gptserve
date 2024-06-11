#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QThread>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDataStream>
#include <QSqlRecord>
class tcpsocket :  public QObject
{
    Q_OBJECT
public:
    explicit tcpsocket(qintptr socketdescripter);
   inline static int numid=0;
    void runwork();
private:
    void receiverequest(QJsonDocument doc);
    ~tcpsocket();
    void userlogin(QJsonObject o);
    void useraskcontent(QJsonObject o);
    void userneedhistorymess(QJsonObject);
    typedef struct
    {
        int size;
        QJsonDocument doc;

    }tcpdata;
private:
    QTcpSocket *socket;
    qintptr des;
    QSqlDatabase db;
    QNetworkAccessManager *manager;
    QSqlQuery query;
    QByteArray date;
    int id;
    QString account;
    int offsethistory=0;
    QString midchat;
signals:
  void disconnectsocket();
    void whathappen(QString);


};

#endif // TCPSOCKET_H
