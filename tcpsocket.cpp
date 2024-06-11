#include "tcpsocket.h"

tcpsocket::tcpsocket(qintptr socketdescripter)
    :des(socketdescripter)
{
  //把tcp db的创建放到run  如果在这里就会发生主线程创建的tcp db 他们会和主线程关联 导致他们会使用主线程的事件循环
}

void tcpsocket::runwork()
{
    socket=new QTcpSocket;
    numid++;
    socket->setSocketDescriptor(des);
    QString connectionName = QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    db=QSqlDatabase::addDatabase("QMYSQL",connectionName);
    db.setHostName("139.196.150.195");
    db.setDatabaseName("userlogin");
    db.setUserName("LiuXin");
    db.setPassword("a320926B!");
    db.setPort(3306);
    db.open();
    emit whathappen(db.lastError().text());
    connect(socket,&QTcpSocket::readyRead,this,[=](){
        QJsonDocument doc;
        date.append(socket->readAll());
        doc=QJsonDocument::fromJson(date);
        if(!doc.isNull()){
            receiverequest(doc);
            date.clear();
        }
    });
    connect(socket,&QTcpSocket::disconnected,this,[=](){
        deleteLater();
        emit whathappen("over");
        emit disconnectsocket();
    });

    manager=new QNetworkAccessManager;
    if(!db.open())qDebug()<<db.lastError().text();
}
void tcpsocket::receiverequest(QJsonDocument doc){
    emit whathappen("new message");
    QJsonObject obj=doc.object();
    QString type=obj["type"].toString();
    if(type=="login")userlogin(obj);

    if(type=="chat")useraskcontent(obj);

    if(type=="history")userneedhistorymess(obj);

}

tcpsocket::~tcpsocket()
{
    qDebug()<<"delete myself";
    numid--;
}
//用户登录函数
void tcpsocket::userlogin(QJsonObject obj)
{
    query= QSqlQuery(db);
    query.prepare("select u,account,password from admin where account = :account and password = :password");
    query.bindValue(":account",obj["account"].toString());
    query.bindValue(":password",obj["password"].toString());
    query.exec();
    emit whathappen(query.lastError().text());
    QJsonObject login;
    login["type"]="login";
    if(query.next()){
        id = query.record().value(0).toInt();
        account = query.record().value(1).toString();
        login["content"]=true;
    }else {
        login["content"]=false;
    }
    socket->write(QJsonDocument(login).toJson()+"LxTcpOverTag");
}
//用户使用函数
void tcpsocket::useraskcontent(QJsonObject obj)
{
    QString content = obj["data"].toObject()
                          ["messages"].toArray()
                                  .last().toObject()
                                  ["content"].toString();
    query.prepare("insert into chatmessages(user_id , message_content ,peoormac)values(:user_id,:messagecontent,:p)");
    query.bindValue(":user_id",id);
    query.bindValue(":messagecontent",content);
    query.bindValue(":p",0);
    query.exec();
    emit whathappen("user say : "+content);
    QNetworkRequest r(QUrl("https://api.aigcapi.io/v1/chat/completions"));
    r.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    r.setRawHeader("Authorization","sk-KoGGLsgRmJcnGbNR5412722fE3264a8e875eB5D177118dA3");
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
    QNetworkReply * reply = manager->post(r,QJsonDocument(obj["data"].toObject()).toJson());
    connect(reply,&QNetworkReply::readyRead,reply,[=](){
        if (reply) {
            QByteArray data = reply->readAll();
            QList<QByteArray> lines = data.split('\n');
            for (const QByteArray &line : lines) {
                if (line.startsWith("data: ")) {
                    QByteArray jsonData = line.mid(6).trimmed();
                    if (!jsonData.isEmpty() && jsonData != "[DONE]") {
                        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
                        if (doc.isObject()) {
                            QJsonObject obj = doc.object();
                            QJsonArray choices = obj["choices"].toArray();
                            for (const QJsonValue &choice : choices) {
                                QJsonObject delta = choice.toObject()["delta"].toObject();
                                if (delta.contains("content")) {
                                    QString chatContent = delta["content"].toString();
                                 // 构建新的 JSON 对象发送到客户端或处理
                                    QJsonObject newObject = obj;
                                    newObject.insert("type", "chat");
                                    midchat.push_back(chatContent);
                                    emit whathappen("openai:" + chatContent);
                                    socket->write(QJsonDocument(newObject).toJson()+"LxTcpOverTag");
                                    socket->waitForBytesWritten();
                                }
                            }
                        }
                    }else {
                        query.prepare("INSERT INTO chatmessages(user_id, message_content, peoormac) VALUES (:user_id, :messagecontent, :p)");
                        query.bindValue(":user_id", id);
                        query.bindValue(":messagecontent", midchat);
                        query.bindValue(":p", 1);
                        query.exec();
                        midchat.clear();
                        qDebug()<<jsonData;

                    }
                }
            }
        }
    });
}
//发送历史询问记录
void tcpsocket::userneedhistorymess(QJsonObject)
{
    QJsonObject data;
    data["type"]="history";
    query.prepare("select message_content,peoormac from chatmessages where user_id = :user_id order by timestamp desc limit :o , :p");
    query.bindValue(":user_id",id);
    query.bindValue(":o",offsethistory);
    query.bindValue(":p",100);
    QJsonArray arr;
    if(query.exec()){
        while(query.next()){
            QJsonObject o;
            o["chat"]=query.value(0).toString();
            o["peoormac"]=query.value(1).toInt();
            arr.append(o);
            offsethistory++;
        }
        data["content"]=arr;
        offsethistory++;
    }else {
        qDebug()<<query.lastError();
    }
    socket->write(QJsonDocument(data).toJson()+"LxTcpOverTag");
}

