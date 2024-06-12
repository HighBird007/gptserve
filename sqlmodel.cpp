#include "sqlmodel.h"

sqlmodel::sqlmodel(QObject *parent):QObject(parent) {

}

void sqlmodel::initsql(QString connectionName)
{
    db=QSqlDatabase::addDatabase("QMYSQL",connectionName);
    db.setHostName("139.196.150.195");
    db.setDatabaseName("userlogin");
    db.setUserName("LiuXin");
    db.setPassword("a320926B!");
    db.setPort(3306);
    db.open();
}

bool sqlmodel::userTryLogin(QString account, QString password)
{
    QSqlQuery query(db);
    query.prepare("select u,account,password from admin where account = :account and password = :password");
    query.bindValue(":account",account);
    query.bindValue(":password",password);
    query.exec();
    if(query.next()){
        id=query.record().value(0).toInt();
        this->account=account;
        return true;
    }else {
        return false;
    }
}

int sqlmodel::getUserId()
{
    return id;
}

void sqlmodel::insertUserChat(QString content)
{
    QSqlQuery query(db);
    query.prepare("insert into chatmessages(user_id , message_content ,peoormac)values(:user_id,:messagecontent,:p)");
    query.bindValue(":user_id",id);
    query.bindValue(":messagecontent",content);
    query.bindValue(":p",0);
    query.exec();
}

void sqlmodel::updateUserUsage()
{
    QString currentDate = QDate::currentDate().toString("yyyy-MM-dd");
    QSqlQuery q(db);
    q.prepare("INSERT INTO chat_usage (account,chat_count,DATE) "
              "VALUES (:user_id, 1, :date) "
              "ON DUPLICATE KEY UPDATE chat_count = chat_count + 1");
    q.bindValue(":user_id", account);
    q.bindValue(":date", currentDate);
    if (!q.exec()) {
        qDebug() << "Error incrementing chat usage:" << q.lastError().text();
    } else {
        qDebug() << "Chat usage incremented successfully for user" << account;
    }
}

void sqlmodel::insertGptChat(QString midchat)
{   QSqlQuery query(db);
    query.prepare("INSERT INTO chatmessages(user_id, message_content, peoormac) VALUES (:user_id, :messagecontent, :p)");
    query.bindValue(":user_id", id);
    query.bindValue(":messagecontent", midchat);
    query.bindValue(":p", 1);
    query.exec();
}

QJsonArray sqlmodel::getHisMess(QJsonObject)
{    QSqlQuery query(db);
    query.prepare("select message_content,peoormac from chatmessages where user_id = :user_id order by timestamp desc limit :p");
    query.bindValue(":user_id",id);
    query.bindValue(":p",100);
    QJsonArray arr;
    query.exec();
        while(query.next()){
            QJsonObject o;
            o["chat"]=query.value(0).toString();
            o["peoormac"]=query.value(1).toInt();
            arr.append(o);
        }
        return arr;

}


