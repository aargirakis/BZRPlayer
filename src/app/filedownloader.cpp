#include "filedownloader.h"

FileDownloader::FileDownloader(QUrl url, QObject* parent) :
    QObject(parent)
{
    connect(
        &m_WebCtrl, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(fileDownloaded(QNetworkReply*))
    );

    // connect(
    //  &m_WebCtrl, SIGNAL (errorOccurred(QNetworkReply*)),
    //  this, SLOT (errorOccured(QNetworkReply::NetworkError*))
    // );

    this->url = url;
    QNetworkRequest request(url);
    m_WebCtrl.get(request);
}

FileDownloader::~FileDownloader()
{
}

void FileDownloader::fileDownloaded(QNetworkReply* pReply)
{
    m_DownloadedData = pReply->readAll();
    //qDebug() << "downloaded!";
    //emit a signal

    pReply->deleteLater();
    emit downloaded();
}

void FileDownloader::errorOccured(QNetworkReply::NetworkError code)
{
    qDebug() << "error!!";
    //emit a signal
}

QByteArray FileDownloader::downloadedData() const
{
    return m_DownloadedData;
}

QUrl FileDownloader::getUrl() const
{
    return url;
}
