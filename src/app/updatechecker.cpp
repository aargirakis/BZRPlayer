#include <QNetworkReply>

#include "updatechecker.h"

// use of GitHub APIs is avoided due to their requests limit

UpdateChecker::UpdateChecker(const QString &currentVersion, const QString &githubRepoUrl, QObject *parent)
    : QObject(parent), currentVer(QVersionNumber::fromString(currentVersion)), githubRepoUrl(githubRepoUrl) {
    manager = new QNetworkAccessManager(this);
}

UpdateChecker::~UpdateChecker() {
    if (reply) {
        reply = nullptr;
    }
}

void UpdateChecker::checkForUpdates() {
    if (currentVer.isNull()) {
        emit checkDone("", "Invalid version provided");
        return;
    }

    if (githubRepoUrl.isEmpty()) {
        emit checkDone("", "No URL provided");
        return;
    }

    const QUrl url(githubRepoUrl + "/releases/latest");

    if (!url.isValid()) {
        emit checkDone("", "Invalid URL provided " + url.toString());
        return;
    }

    QNetworkRequest request(url);
    request.setTransferTimeout(30000);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QString(
                          "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36"));

    reply = manager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this] {
        reply->deleteLater();

        if (const auto error = reply->error();
            error != QNetworkReply::NoError) {
            emit checkDone("", "Network error: " + reply->errorString());
            return;
        }

        if (const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
            status.isValid()) {
            if (const auto code = status.toInt(); code >= 400) {
                const auto reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
                emit checkDone(
                    "", QString("HTTP Error %1: %2").arg(code).arg(reason.isEmpty() ? "Unknown Error" : reason));
                return;
            }
        }

        const auto path = reply->url().path();
        QString remoteVerStr;

        if (path.contains("/releases/tag/")) {
            if (const auto segments = path.split("/", Qt::SkipEmptyParts);
                !segments.isEmpty()) {
                remoteVerStr = segments.last();
            }
        }

        const auto remoteVer = QVersionNumber::fromString(remoteVerStr);

        if (remoteVer.isNull()) {
            emit checkDone("", "Redirect URL is not valid");
            return;
        }

        if (remoteVer > currentVer) {
            emit checkDone(remoteVerStr, "");
        }
    });
}
