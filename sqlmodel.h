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
    void insertUserChat(QString,int);
    void updateUserUsage();
    void insertGptChat(QString, int currentid);
    QJsonArray getHisMess(QJsonObject);
    QJsonArray getChatLabels();
    void createNewTag();
    void deleteTag(int i);
private:
    QSqlDatabase db;
    int id=-1;
    QString account;
};

#endif // SQLMODEL_H
