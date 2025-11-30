#include "httpclient.hpp"

HttpClient::HttpClient(QObject* parent)
    : QObject(parent), manager(new QNetworkAccessManager(this)) {};
HttpClient::HttpClient(QObject* parent, const QMap<QString, QString>& headers)
    : QObject(parent), manager(new QNetworkAccessManager(this)), headers(headers) {};
HttpClient::~HttpClient() {
    delete manager;
}

void HttpClient::setRootCA(const QString& certPath) {
    QFile file(certPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Unable to load root certificate" << file.errorString();
        return;
    }

    const QByteArray bytes = file.readAll();
    const QSslCertificate certificate(bytes);

    // Add custom ca to default ssl configuration
    QSslConfiguration configuration = QSslConfiguration::defaultConfiguration();
    auto certs = configuration.caCertificates();
    certs.append(certificate);

    configuration.setCaCertificates(certs);
    QSslConfiguration::setDefaultConfiguration(configuration);
    file.close();
}

void HttpClient::setBearerToken(const QString& jwtToken) {
    HttpClient::token = jwtToken;
}

// Initialize static token
QString HttpClient::token = QString();

void HttpClient::get(const QString& url) noexcept {
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
    setHeaders(&request);
    QNetworkReply* reply = manager->get(request);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onReplyFinished);
}

void HttpClient::post(const QString& url, const QByteArray& data) noexcept {
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
    setHeaders(&request);
    QNetworkReply* reply = manager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onReplyFinished);
}

void HttpClient::put(const QString& url, const QByteArray& data) noexcept {
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
    setHeaders(&request);
    QNetworkReply* reply = manager->put(request, data);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onReplyFinished);
}

void HttpClient::patch(const QString& url, const QByteArray& data) noexcept {
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
    setHeaders(&request);
    QNetworkReply* reply = manager->sendCustomRequest(request, "PATCH", data);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onReplyFinished);
}

void HttpClient::del(const QString& url) noexcept {
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
    setHeaders(&request);

    QNetworkReply* reply = manager->deleteResource(request);
    connect(reply, &QNetworkReply::finished, this, &HttpClient::onReplyFinished);
}

void HttpClient::onReplyFinished() {
    auto* reply = qobject_cast<QNetworkReply*>(sender());
    bool requestFailed = reply->error() != QNetworkReply::NoError;

    QByteArray responseData = reply->readAll();
    QString connError = reply->errorString();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    reply->deleteLater();

    const bool ok = !requestFailed && statusCode >= 200 && statusCode < 300;

    HttpResponse response{.OK = ok,
                          .statusCode = statusCode,
                          .data = ok ? responseData : QByteArray(),
                          .errorString = ok ? QString() : responseData};

    // We have a non-HTTP error
    if (statusCode == 0) {
        response.errorString = connError;
    }

    emit finished(response);
}

HttpResponse HttpClient::get_sync(const QString& url) {
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
    setHeaders(&request);
    QNetworkReply* reply = manager->get(request);
    return waitForResponse(reply);
}

HttpResponse HttpClient::post_sync(const QString& url, const QByteArray& data) {
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
    setHeaders(&request);
    QNetworkReply* reply = manager->post(request, data);
    return waitForResponse(reply);
}

HttpResponse HttpClient::put_sync(const QString& url, const QByteArray& data) {
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
    setHeaders(&request);
    QNetworkReply* reply = manager->put(request, data);
    return waitForResponse(reply);
}

HttpResponse HttpClient::patch_sync(const QString& url, const QByteArray& data) {
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
    setHeaders(&request);
    QNetworkReply* reply = manager->sendCustomRequest(request, "PATCH", data);
    return waitForResponse(reply);
}

HttpResponse HttpClient::del_sync(const QString& url) {
    QUrl qUrl(url);
    QNetworkRequest request(qUrl);
    setHeaders(&request);

    QNetworkReply* reply = manager->deleteResource(request);
    return waitForResponse(reply);
}

void HttpClient::setHeaders(QNetworkRequest* request) {
    // Set all request headers
    QMapIterator<QString, QString> it(headers);
    while (it.hasNext()) {
        it.next();
        request->setRawHeader(it.key().toLocal8Bit(), it.value().toLocal8Bit());
    }

    // Add authorization header if token is set
    if (!token.isEmpty()) {
        request->setRawHeader("Authorization", QString("Bearer ").append(token).toLocal8Bit());
    }
}

HttpResponse HttpClient::waitForResponse(QNetworkReply* reply) {
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    bool requestFailed = reply->error() != QNetworkReply::NoError;
    QString connError = reply->errorString();
    QByteArray responseData = reply->readAll();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    // Free reply memory
    reply->deleteLater();

    const bool ok = !requestFailed && statusCode >= 200 && statusCode < 300;

    HttpResponse response{.OK = ok,
                          .statusCode = statusCode,
                          .data = ok ? responseData : QByteArray(),
                          .errorString = ok ? QString() : responseData};

    // We have a non-HTTP error
    if (statusCode == 0) {
        response.errorString = connError;
    }
    return response;
}

void writeFile(const QString& path, const QByteArray& data) {
    auto* file = new QFile(path);
    if (file->open(QIODevice::WriteOnly)) {
        file->write(data);
        file->close();
    }
}

QImage imageFromBytes(const QByteArray& data) {
    QImageReader imageReader(data);
    QImage img = imageReader.read();
    return img;
}
