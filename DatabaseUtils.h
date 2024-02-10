#ifndef DATABASE_UTILS_H
#define DATABASE_UTILS_H

#include <QDebug>
#include <QMap>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <functional>
#include <utility>

class QueryCommand {
   public:
    QueryCommand(QSqlQuery& query);
    virtual ~QueryCommand();
    virtual std::pair<bool, QString> execute(std::function<void(QSqlQuery&)> processResults);
    virtual void bindParameters();
    void bindParam(const QString& paramName, const QVariant& value);
    QString lastError() const;

   protected:
    QMap<QString, QVariant> params;
    QSqlQuery& query;
};

class Query : public QueryCommand {
   public:
    Query(QSqlQuery& query, const QString& queryString);
    ~Query();

    void bindParameters() override;
    std::pair<bool, QString> execute(std::function<void(QSqlQuery&)> processResults) override;

   private:
    QString queryString;
};

// Helper function for connecting to a MySQL database
bool connectToMySQLDatabase(const QString& databaseName, const QString& username = "root", const QString& host = "127.0.0.1", int port = 3306,
                            const QString& password = "") {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName(databaseName);
    db.setUserName(username);
    db.setPassword(password);

    if (!db.open()) {
        qDebug() << "Failed to connect to MySQL database:";
        qDebug() << db.lastError().text();
        return false;
    }

    qDebug() << "Connected to MySQL database!";
    return true;
}

// Helper function for connecting to an SQLite database
bool connectToSQLiteDatabase(const QString& databasePath) {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(databasePath);

    if (!db.open()) {
        qDebug() << "Failed to connect to SQLite database:";
        qDebug() << db.lastError().text();
        return false;
    }

    qDebug() << "Connected to SQLite database!";
    return true;
}

// Helper function for connecting to a PostgreSQL database
bool connectToPostgreSQLDatabase(const QString& databaseName, const QString& username = "root", const QString& host = "127.0.0.1", int port = 5432,
                                 const QString& password = "") {
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName(databaseName);
    db.setUserName(username);
    db.setPassword(password);

    if (!db.open()) {
        qDebug() << "Failed to connect to PostgreSQL database:";
        qDebug() << db.lastError().text();
        return false;
    }

    qDebug() << "Connected to PostgreSQL database!";
    return true;
}

#endif  // DATABASE_UTILS_H
