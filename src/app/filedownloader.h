#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QNetworkReply>

class FileDownloader : public QObject {
    Q_OBJECT

public:
    explicit FileDownloader(const QUrl &url, QObject *parent = nullptr);

    virtual ~FileDownloader();

    QByteArray downloadedData() const;

    QUrl getUrl() const;


signals:
    void downloaded();

private slots:
    void fileDownloaded(QNetworkReply *pReply);

    static void errorOccured(QNetworkReply::NetworkError);

private:
    QNetworkAccessManager m_WebCtrl;
    QByteArray m_DownloadedData;
    QUrl url;
};

#endif // FILEDOWNLOADER_H
