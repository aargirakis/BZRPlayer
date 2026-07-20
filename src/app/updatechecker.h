#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QNetworkAccessManager>
#include <QVersionNumber>

class UpdateChecker : public QObject {
    Q_OBJECT

public:
    explicit UpdateChecker(const QString &currentVersion, const QString &githubRepoUrl, QObject *parent = nullptr);

    ~UpdateChecker();

    void checkForUpdates();

signals:
    void checkDone(const QString &version, const QString &error);

private:
    QNetworkAccessManager *manager;
    QNetworkReply *reply = nullptr;
    QVersionNumber currentVer;
    QString githubRepoUrl;
};

#endif // UPDATECHECKER_H
