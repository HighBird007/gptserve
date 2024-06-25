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

void sqlmodel::insertUserChat(QString content,int currentid)
{
    QSqlQuery query(db);
    query.prepare("INSERT INTO chatmessages(user_id,chatLabelId, message_content, peoormac) VALUES (:user_id,:chatId,:messagecontent, :p)");
    query.bindValue(":user_id", id);
    query.bindValue(":messagecontent", content);
    query.bindValue(":chatId", currentid);
    query.bindValue(":p", 0);
    query.exec();
    qDebug()<<query.lastError();
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

void sqlmodel::insertGptChat(QString midchat,int currentid)
{   QSqlQuery query(db);
    query.prepare("INSERT INTO chatmessages(user_id,chatLabelId, message_content, peoormac) VALUES (:user_id,:chatId,:messagecontent, :p)");
    query.bindValue(":user_id", id);
    query.bindValue(":messagecontent", midchat);
    query.bindValue(":chatId", currentid);
    query.bindValue(":p", 1);
    query.exec();
}

QJsonArray sqlmodel::getHisMess(QJsonObject o)
{    QSqlQuery query(db);
    query.prepare("select chatLabelId,message_content,peoormac from chatmessages where user_id = :user_id and chatLabelId =:labelId order by timestamp desc ");
    query.bindValue(":user_id",id);
    query.bindValue(":labelId",o["chatId"].toInt());
    QJsonArray arr;
    query.exec();
    qDebug()<<query.lastError();
        while(query.next()){
            QJsonObject o;
            o["chat"]=query.value(1).toString();
            o["peoormac"]=query.value(2).toInt();
            arr.append(o);
            qDebug()<<o;
        }
        return arr;

}

QJsonArray sqlmodel::getChatLabels()
{
    QJsonArray arr;
    QSqlQuery query(db);
    query.prepare("select userAccount,chatLabelId,chatLabelContent from userchatlabels where userAccount= :user_id order by timestamp desc");
    query.bindValue(":user_id",account);
    query.exec();
    while(query.next()){
        QJsonObject o;
        o["chatLabelId"] = query.record().value(1).toInt();
        o["chatLabelContent"] = query.record().value(2).toString();
        arr.append(o);
    }
    return arr;
}

void sqlmodel::createNewTag()
{
    QSqlQuery query(db);
    query.prepare("insert into userchatlabels(userAccount, chatLabelContent)values (:account,:content)");
    query.bindValue(":content",QTime::currentTime().toString());
    query.bindValue(":account",account);
    query.exec();
    qDebug()<<query.lastError();
}

void sqlmodel::deleteTag(int i)
{
    qDebug()<<i;
    QSqlQuery query(db);
    query.prepare("delete from chatmessages where user_id =:user AND chatLabelId =:id");
    query.bindValue(":user",id);
    query.bindValue(":id",i);
    query.exec();
    query.prepare("delete from userchatlabels where userAccount =:user AND chatLabelId =:id");
    query.bindValue(":user",account);
    query.bindValue(":id",i);
    query.exec();
}


