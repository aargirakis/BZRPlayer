#include <QDir>
#include <QNetworkReply>

#include "filesdownloader.h"

FilesDownloader::FilesDownloader(const QList<QUrl> &urls, const QString &destDir, QObject *parent)
    : QObject(parent), urls(urls), targetDir(destDir != "" ? QDir(destDir).absolutePath() : ""), completedCount(0) {
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &FilesDownloader::onDownloadsDone);
}

void FilesDownloader::downloadFiles() {
    if (urls.isEmpty()) {
        emit downloadsFinished("No URL provided");
        return;
    }

    if (targetDir.isEmpty()) {
        emit downloadsFinished("No destination directory provided");
        return;
    }

    if (const QDir dir(targetDir); !dir.exists() && !dir.mkpath(".")) {
        emit downloadsFinished("Unable to create directory " + targetDir);
        return;
    }

    for (const QUrl &url: urls) {
        if (!url.isValid()) {
            emit downloadsFinished("Invalid URL provided " + url.toString());
            return;
        }

        if (const auto filename = url.fileName(); filename.isEmpty()) {
            emit downloadsFinished("No file to download at " + url.toString());
            return;
        }

        QNetworkRequest request(url);
        request.setTransferTimeout(30000);

        QNetworkReply *reply = manager->get(request);
        activeReplies.append(reply);

        connect(reply, &QNetworkReply::finished, this, [reply] { reply->deleteLater(); });
    }
}

FilesDownloader::~FilesDownloader() {
    for (QNetworkReply *reply: activeReplies) {
        if (reply) reply->abort();
    }

    activeReplies.clear();
}

void FilesDownloader::onDownloadsDone(QNetworkReply *reply) {
    const auto urlCurrent = reply->url();
    activeReplies.removeAll(reply);

    QString error;

    if (reply->error() != QNetworkReply::NoError) {
        error = reply->errorString();
    } else if (const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute); status.isValid()) {
        if (const auto code = status.toInt(); code >= 400) {
            const auto reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
            error = QString("HTTP Error %1: %2").arg(code).arg(reason.isEmpty() ? "Unknown Error" : reason);
        }
    }

    if (error.isEmpty()) {
        dataByUrl[urlCurrent] = reply->readAll();
    } else {
        errors[urlCurrent] = error;
    }

    completedCount++;

    if (completedCount != urls.size()) {
        return;
    }

    if (!errors.isEmpty()) {
        const auto msg = QString("%1/%2 downloads are failed. First error: %3")
                .arg(errors.size())
                .arg(urls.size())
                .arg(errors.begin().value());

        emit downloadsFinished(msg);
        return;
    }

    for (auto resultsIterator = dataByUrl.begin(); resultsIterator != dataByUrl.end(); ++resultsIterator) {
        const auto filePath = QDir(targetDir).filePath(resultsIterator.key().fileName());
        QFile file(filePath);

        if (!file.open(QIODevice::WriteOnly)) {
            emit downloadsFinished("Couldn't write to file " + filePath);
            return;
        }

        if (const auto &data = resultsIterator.value(); file.write(data) != data.size()) {
            file.close();
            file.remove();
            emit downloadsFinished("Corrupted file " + filePath);
            return;
        }

        file.close();
    }

    emit downloadsFinished("");
}
