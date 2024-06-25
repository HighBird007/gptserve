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
#include <sqlmodel.h>
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
    void userNeedChatLabels();
    void userCreateTag();
    void userDeleteTag(QJsonObject);
private:
    QTcpSocket *socket;
    qintptr des;
    QNetworkAccessManager *manager;
    QSqlQuery query;
    QByteArray date;
    int id;
    QString account;
    QString midchat;
    sqlmodel *sql;
    int currentchatid;
signals:
  void disconnectsocket();
    void whathappen(QString);


};

#endif // TCPSOCKET_H
