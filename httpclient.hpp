#ifndef __HTTPCLIENT_H__
#define __HTTPCLIENT_H__

#include <QFile>
#include <QImageReader>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QUrl>

struct HttpResponse {
    bool OK;              // OK is true if request was successful.
    int statusCode;       // StatusCode
    QByteArray data;      // Contains response body if OK
    QString errorString;  // Contains errorString if not OK
};

/**
 * @brief HttpClient is a wrapper around the QNetworkAccessManager to simplify
 * performing http requests in Qt.
 *
 * It supports both syncronous and asyncronous methods.
 * All syncronous have a suffix of _sync and throw a NetworkException if the request fails
 * or the status code is > 300.
 *
 * The asyncronous methods use signals success and error to communicate when data is ready
 * or when an error occurs. The error returned is read from the request body as a QByteArray.
 *
 * Each request uses the same QNetworkAccessManager instance but different QNetwork object.
 * This means you can use the same client to perform multiple subsequent requests.
 */
class HttpClient : public QObject {
    Q_OBJECT

   public:
    /**
     * @brief Construct a new Http Client object
     *
     * @param parent *QObject
     */
    HttpClient(QObject* parent = nullptr);

    /**
     * @brief Construct a new Http Client object with parent and default http headers.
     * The headers are a QMap of strings and are automatically added to every request.
     *
     * @param parent *QObject
     * @param headers QMap<QString, QString>
     */
    HttpClient(QObject* parent, const QMap<QString, QString>& headers);

    /**
     * @brief Destroy the Http Client object
     *
     */
    virtual ~HttpClient();

    /**
     * @brief Set the Root CA object
     *
     * @param certPath QString
     */
    static void setRootCA(const QString& certPath);

    /**
     * @brief Set the Bearer Token string. This will be used to Bearer Auth.
     *
     * @param jwtToken QString.
     */
    static void setBearerToken(const QString& jwtToken);

    /**
     * @brief Perform a GET request asyncronously. You will need to access the response by connecting
     * to the success signal and error to error signal.
     *
     * @param url QString
     */
    void get(const QString& url) noexcept;

    /**
     * @brief Perform a POST request asyncronously. You will need to access the response by connecting
     * to the success signal and error to error signal.
     *
     * @param url QString
     * @param data QByteArray
     */
    void post(const QString& url, const QByteArray& data) noexcept;

    /**
     * @brief Perform a PUT request asyncronously. You will need to access the response by connecting
     * to the success signal and error to error signal.
     *
     * @param url QString
     * @param data QByteArray
     */
    void put(const QString& url, const QByteArray& data) noexcept;

    /**
     * @brief Perform a PATCH request asyncronously. You will need to access the response by connecting
     * to the success signal and error to error signal.
     *
     * @param url QString
     * @param data QByteArray
     */
    void patch(const QString& url, const QByteArray& data) noexcept;

    /**
     * @brief Perform a DELETE request asyncronously. You will need to access the response by connecting
     * to the success signal and error to error signal.
     *
     * @param url QString
     */
    void del(const QString& url) noexcept;

    /** Perform syncronous GET request and block until the response arrives
     * Returns data in request body if successful or throws a NetworkException if it fails.
     * You must catch this error to avoid segmentation faults.
     */
    HttpResponse get_sync(const QString& url);

    /** Perform syncronous POST request and block until the response arrives
     * Returns data in request body if successful or throws a NetworkException if it fails.
     * You must catch this error to avoid segmentation faults.
     */
    HttpResponse post_sync(const QString& url, const QByteArray& data);

    /** Perform syncronous PUT request and block until the response arrives
     * Returns data in request body if successful or throws a NetworkException if it fails.
     * You must catch this error to avoid segmentation faults.
     */
    HttpResponse put_sync(const QString& url, const QByteArray& data);

    /** Perform syncronous PATCH request and block until the response arrives
     * Returns data in request body if successful or throws a NetworkException if it fails.
     * You must catch this error to avoid segmentation faults.
     */
    HttpResponse patch_sync(const QString& url, const QByteArray& data);

    /** Perform syncronous DELETE request and block until the response arrives
     * Returns data in request body if successful or throws a NetworkException if it fails.
     * You must catch this error to avoid segmentation faults.
     */
    HttpResponse del_sync(const QString& url);

   private:
    QNetworkAccessManager* manager;
    QMap<QString, QString> headers;
    void setHeaders(QNetworkRequest* request);

    static QString token;  // The JWT

    // Used by all syncronous method to process reply, read data and return it to caller
    // and is responsible for throwing the NetworkException is the reply failed or status
    // code is > 300.
    HttpResponse waitForResponse(QNetworkReply* reply);

   signals:

    // Signal emitted when the request finishes. The response contains the status
    // of the request, data and error(if the request failed)
    void finished(const HttpResponse& response);

   private slots:

    /**
     * @brief Slot for internal use to process network reply.
     *
     */
    void onReplyFinished();
};

void writeFile(const QString& path, const QByteArray& data);
QImage imageFromBytes(const QByteArray& data);

#endif /* __HTTPCLIENT_H__ */
