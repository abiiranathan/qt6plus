
#include "DatabaseUtils.h"

int main() {
    // Set up the database connection
    if (!connectToSQLiteDatabase(":memory:")) {
        return 1;
    }

    // Create the user table in the database
    QSqlQuery createQuery;
    if (!createQuery.exec("CREATE TABLE users ("
                          "name TEXT, "
                          "age INTEGER, "
                          "sex TEXT)")) {
        qDebug() << "Error creating table:" << createQuery.lastError().text();
        return 1;
    }

    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO users(name,age,sex) VALUES (:name, :age, :sex)");
    insertQuery.bindValue(":name", "Abiira");
    insertQuery.bindValue(":age", 28);
    insertQuery.bindValue(":sex", "Male");

    if (!insertQuery.exec()) {
        qDebug() << "Error inserting data:" << insertQuery.lastError().text();
    }

    // Using the query helper.
    QSqlQuery q;
    Query selectQuery(q, "SELECT * FROM users WHERE name = :name");
    selectQuery.bindParam("name", "Abiira");

    auto [success, errorMessage] = selectQuery.execute([](QSqlQuery& query) {
        while (query.next()) {
            QString column1 = query.value(0).toString();
            int column2 = query.value(1).toInt();
            QString column3 = query.value(2).toString();
            qDebug() << column1 + " " + QString::number(column2) + " " + column3;
        }
    });

    if (!success) {
        qDebug() << "Failed to execute query:" << errorMessage;
    }

    QSqlDatabase::database().close();
    return 0;
}
