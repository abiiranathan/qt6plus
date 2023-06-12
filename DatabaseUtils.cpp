#include "DatabaseUtils.h"

// Base implementation ============================================
QueryCommand::~QueryCommand() = default;
QueryCommand::QueryCommand(QSqlQuery& query)
    : query(query) {}

std::pair<bool, QString> QueryCommand::execute(std::function<void(QSqlQuery&)> processResults) {
    return std::make_pair(false, QString());
}

void QueryCommand::bindParameters() {}
void QueryCommand::bindParam(const QString& paramName, const QVariant& value) {
    params[paramName] = value;
}

QString QueryCommand::lastError() const {
    return query.lastError().text();
}

// Fetch command implementations =======================================
Query::Query(QSqlQuery& query, const QString& queryString)
    : QueryCommand(query), queryString(queryString) {}

Query::~Query() = default;

void Query::bindParameters() {
    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        const QString& paramName = it.key();
        const QVariant& value = it.value();
        query.bindValue(":" + paramName, value);
    }
}

std::pair<bool, QString> Query::execute(std::function<void(QSqlQuery&)> processResults) {
    if (!query.prepare(queryString)) {
        goto fail;
    }

    bindParameters();

    if (!query.exec()) {
        goto fail;
    }

    if (processResults) {
        processResults(query);
    }

    return std::make_pair(true, QString());

fail:
    QString errorMessage = "Failed to execute query: " + lastError();
    return std::make_pair(false, errorMessage);
}
