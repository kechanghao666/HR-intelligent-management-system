#ifndef XFYUN_API_H
#define XFYUN_API_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QSslError>

class XfyunAPI : public QObject
{
    Q_OBJECT
public:
    explicit XfyunAPI(QObject *parent = nullptr);

    void askQuestion(const QString &question);

signals:
    void answerReceived(const QString &answer);

private slots:
    void onReplyFinished(QNetworkReply *reply);
    void onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);

private:
    void initializeSSLConfiguration();
    void setupNetworkSecurity();
    bool isSSLSupported();
    QString generateLocalResponse(const QString& question);
    void loadCredentials();
    
    QNetworkAccessManager *manager;
    // 记录最近一次提问内容，用于失败时本地兜底回答
    QString lastQuestion;
    // WebSocket鉴权使用（保留）
    QString appId = "a473258b";
    QString apiSecret = "MjZmZmMzNTI3Y2JjYjdhOWM0ODVjNTEx";
    QString apiKey = "3cad5777de1da718630f606cfc223a56";
    // HTTP REST 使用：Authorization: Bearer {apiPassword}
    QString apiPassword; // 可从QSettings/env载入
};

#endif