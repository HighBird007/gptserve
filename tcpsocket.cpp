#include "tcpsocket.h"

tcpsocket::tcpsocket(qintptr socketdescripter)
    :des(socketdescripter)
{
  //把tcp db的创建放到run  如果在这里就会发生主线程创建的tcp db 他们会和主线程关联 导致他们会使用主线程的事件循环
}

void tcpsocket::runwork()
{
    socket=new QTcpSocket;
    socket->setSocketDescriptor(des);
    QString connectionName = QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    sql =new sqlmodel(this);
    sql->initsql(connectionName);

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
}
//用户登录函数
void tcpsocket::userlogin(QJsonObject obj)
{

    QJsonObject login;
    login["type"]="login";
    if(sql->userTryLogin(obj["account"].toString(),obj["password"].toString())){
        id = sql->getUserId();
        account = obj["account"].toString();
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
    sql->insertUserChat(content);
    emit whathappen("user say : "+content);
    QNetworkRequest r(QUrl("https://api.aigcapi.io/v1/chat/completions"));
    r.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");
    r.setRawHeader("Authorization","sk-KoGGLsgRmJcnGbNR5412722fE3264a8e875eB5D177118dA3");
    sql->updateUserUsage();
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
                                    socket->write(QJsonDocument(newObject).toJson()+"LxTcpOverTag");
                                    socket->waitForBytesWritten();
                                }
                            }
                        }
                    }else {
                        sql->insertGptChat(midchat);
                        emit whathappen("openai:" + midchat);
                        midchat.clear();
                    }
                }
            }
        }
    });
}
//发送历史询问记录
void tcpsocket::userneedhistorymess(QJsonObject o)
{
    QJsonObject data;
    data["type"]="history";
    data["content"]=sql->getHisMess(o);
    socket->write(QJsonDocument(data).toJson()+"LxTcpOverTag");
}

