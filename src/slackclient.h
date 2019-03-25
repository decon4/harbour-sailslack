#ifndef SLACKCLIENT_H
#define SLACKCLIENT_H

#include <QObject>
#include <QPointer>
#include <QImage>
#include <QFile>
#include <QJsonObject>
#include <QUrl>
#include <QTimer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "slackclientconfig.h"
#include "slackstream.h"

class SlackClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(SlackClientConfig* config READ getConfig CONSTANT)
    Q_PROPERTY(QString team READ getTeam CONSTANT)
    Q_PROPERTY(bool initialized READ getInitialized NOTIFY initializedChanged)
public:
    explicit SlackClient(QObject *parent = 0); // only to make qmlRegisterType happy, don't use
    explicit SlackClient(const QString &team, QObject *parent = 0);
    ~SlackClient() override;

    Q_INVOKABLE void setAppActive(bool active);
    Q_INVOKABLE void setActiveWindow(QString windowId);

    Q_INVOKABLE QVariantList getChannels();
    Q_INVOKABLE QVariant getChannel(QString channelId);

    SlackClientConfig *getConfig() const { return this->config; }
    bool getInitialized() const { return initialized; }

    void setTeam(const QString &team);
    QString getTeam() const;

signals:
    void teamChanged();

    void testConnectionFail();
    void testLoginSuccess();
    void testLoginFail();

    void loadUsersSuccess();
    void loadUsersFail();

    void loadMessagesSuccess(QString channelId, QVariantList messages, bool hasMore);
    void loadMessagesFail();
    void loadHistorySuccess(QString channelId, QVariantList messages, bool hasMore);
    void loadHistoryFail();

    void initFail();
    void initSuccess();
    void initializedChanged();

    void reconnectFail();
    void reconnectAccessTokenFail();

    void messageReceived(QVariantMap message);
    void channelUpdated(QVariantMap channel);
    void channelJoined(QVariantMap channel);
    void channelLeft(QVariantMap channel);
    void userUpdated(QVariantMap user);

    void postImageSuccess();
    void postImageFail();

    void networkOff();
    void networkOn();

    void reconnecting();
    void disconnected();
    void connected();

public slots:
    void init();
    void start();
    void reconnect();

    void testLogin();
    void handleTestLoginReply();

    void loadHistory(QString type, QString channelId, QString latest);
    void loadMessages(QString type, QString channelId);
    void handleLoadMessagesReply();

    void logout();
    void loadUsers();
    void markChannel(QString type, QString channelId, QString time);
    void joinChannel(QString channelId);
    void leaveChannel(QString channelId);
    void leaveGroup(QString groupId);
    void openChat(QString chatId);
    void closeChat(QString chatId);
    void postMessage(QString channelId, QString content);
    void postImage(QString channelId, QString imagePath, QString title, QString comment);

    void handleNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);

    void handleStreamStart();
    void handleStreamEnd();
    void handleStreamMessage(QJsonObject message);


private:
    QString team;

    bool appActive;
    QString activeWindow;

    QNetworkReply* executePost(QString method, const QMap<QString, QString> &data);
    QNetworkReply *executePostWithFile(QString method, const QMap<QString, QString>&, QFile *file);

    QNetworkReply* executeGet(QString method, QMap<QString,QString> params = QMap<QString,QString>());

    static QString toString(const QJsonObject &data);

    void loadConversations(QString cursor = QString());

    void parseMessageUpdate(QJsonObject message);
    void parseChannelUpdate(QJsonObject message);
    void parseChannelJoin(QJsonObject message);
    void parseChannelLeft(QJsonObject message);
    void parseGroupJoin(QJsonObject message);
    void parseChatOpen(QJsonObject message);
    void parseChatClose(QJsonObject message);
    void parsePresenceChange(QJsonObject message);
    void parseNotification(QJsonObject message);

    QVariantList parseMessages(const QJsonObject data);
    QVariantMap getMessageData(const QJsonObject message);

    QString getContent(QJsonObject message);
    QVariantList getAttachments(QJsonObject message);
    QVariantList getImages(QJsonObject message);
    QString getAttachmentColor(QJsonObject attachment);
    QVariantList getAttachmentFields(QJsonObject attachment);
    QVariantList getAttachmentImages(QJsonObject attachment);

    QVariantMap parseChannel(QJsonObject data);
    QVariantMap parseGroup(QJsonObject group);
    QVariantMap parseChat(QJsonObject chat);

    void parseUsers(QJsonObject data);
    void findNewUsers(const QString &message);

    void sendNotification(QString channelId, QString title, QString content);
    void clearNotifications();

    QVariantMap user(const QJsonObject &data);

    QString historyMethod(QString type);
    QString markMethod(QString type);

    QPointer<QNetworkAccessManager> networkAccessManager;
    QPointer<SlackClientConfig> config;
    QPointer<SlackStream> stream;
    QPointer<QTimer> reconnectTimer;

    QNetworkAccessManager::NetworkAccessibility networkAccessible;

    bool initialized;
};

class SlackClientFactory : public QObject
{
    Q_OBJECT
public:
    explicit SlackClientFactory(QObject *parent = nullptr);

    Q_INVOKABLE SlackClient *createClient(const QString &team);
};

#endif // SLACKCLIENT_H
