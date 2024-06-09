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
    QJsonObject a;
    a["type"]="error";
    a["content"]="hello";
    socket->write(QJsonDocument(a).toJson());
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
        id =   query.record().value(0).toInt();
        login["content"]=true;
    }else {
        login["content"]=false;
    }
    socket->write(QJsonDocument(login).toJson());
}
//用户使用函数
void tcpsocket::useraskcontent(QJsonObject obj)
{   qDebug()<<obj;
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
    QNetworkReply * reply = manager->post(r,QJsonDocument(obj["data"].toObject()).toJson());
    connect(reply,&QNetworkReply::readyRead,reply,[=](){
        QJsonDocument doc=QJsonDocument::fromJson(reply->readAll());
        QJsonObject o =doc .object();
        qDebug()<<o;
        o.insert("type","chat");
        QJsonArray choicesArray = o["choices"].toArray();
        // 获取第一个选择项
        QJsonObject choiceObject = choicesArray[0].toObject();
        // 获取message对象
        QJsonObject messageObject = choiceObject["message"].toObject();
        // 获取content字段中的聊天内容
        QString chatContent = messageObject["content"].toString();
        qDebug()<<chatContent;
        emit whathappen("openai:"+chatContent);
        query.prepare("insert into chatmessages(user_id , message_content ,peoormac)values(:user_id,:messagecontent,:p)");
        query.bindValue(":user_id",id);
        query.bindValue(":messagecontent",chatContent);
        query.bindValue(":p",1);
        query.exec();
        socket->write(QJsonDocument(o).toJson());
        reply->deleteLater();
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

    socket->write(QJsonDocument(data).toJson());
}

