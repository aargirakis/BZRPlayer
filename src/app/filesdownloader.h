#ifndef FILESDOWNLOADER_H
#define FILESDOWNLOADER_H

#include <QNetworkAccessManager>

class FilesDownloader : public QObject {
    Q_OBJECT

public:
    explicit FilesDownloader(const QList<QUrl> &urls, const QString &destDir, QObject *parent = nullptr);

    ~FilesDownloader();

    void downloadFiles();

signals:
    void downloadsFinished(const QString &errorMessage);

private slots:
    void onDownloadsDone(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
    QList<QNetworkReply *> activeReplies;
    QList<QUrl> urls;
    QString targetDir;
    QMap<QUrl, QByteArray> dataByUrl;
    QMap<QUrl, QString> errors;
    qsizetype completedCount;
};

#endif // FILESDOWNLOADER_H
