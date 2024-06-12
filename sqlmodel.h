#ifndef SQLMODEL_H
#define SQLMODEL_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTime>
#include <QSqlRecord>
#include <QJsonArray>
#include <QJsonObject>
class sqlmodel : public QObject
{
    Q_OBJECT
public:
    sqlmodel(QObject *parent =nullptr);
    void initsql(QString);
    bool userTryLogin(QString,QString);
    int getUserId();
    void insertUserChat(QString);
    void updateUserUsage();
    void insertGptChat(QString);
    QJsonArray getHisMess(QJsonObject);
private:
    QSqlDatabase db;
    int id=-1;
    QString account;
};

#endif // SQLMODEL_H
