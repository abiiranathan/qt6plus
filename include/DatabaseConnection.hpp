#ifndef DATABASE_CONNECTION_H
#define DATABASE_CONNECTION_H

#include "DatabaseOptions.hpp"

#include <QString>
#include <QUuid>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <optional>

/**
 * Database connection manager that handles database connections using ConnOptions.
 * Provides RAII-based connection lifecycle management with proper cleanup.
 * Safe for use in multi-threaded applications when each thread has its own instance.
 */
class DatabaseConnection {
   public:
    /**
     * Constructs a DatabaseConnection with the specified options.
     * Does not establish the connection immediately - call open() to connect.
     * @param options Connection options specifying database type and parameters.
     * @param connectionName Optional custom connection name. If empty, generates unique name.
     */
    explicit DatabaseConnection(ConnOptions options, QString connectionName = QString())
        : m_options(std::move(options)),
          m_connectionName(connectionName.isEmpty() ? generateConnectionName()
                                                    : std::move(connectionName)) {}

    /**
     * Destructor ensures the connection is properly closed.
     * Removes the connection from Qt's connection pool.
     */
    ~DatabaseConnection() { close(); }

    // Disable copying to prevent connection name conflicts
    DatabaseConnection(const DatabaseConnection&) = delete;
    DatabaseConnection& operator=(const DatabaseConnection&) = delete;

    // Enable moving
    DatabaseConnection(DatabaseConnection&& other) noexcept
        : m_options(std::move(other.m_options)),
          m_connectionName(std::move(other.m_connectionName)),
          m_database(other.m_database),
          m_isOpen(other.m_isOpen) {
        other.m_isOpen = false;
    }

    DatabaseConnection& operator=(DatabaseConnection&& other) noexcept {
        if (this != &other) {
            close();
            m_options = std::move(other.m_options);
            m_connectionName = std::move(other.m_connectionName);
            m_database = other.m_database;
            m_isOpen = other.m_isOpen;
            other.m_isOpen = false;
        }
        return *this;
    }

    /**
     * Opens the database connection using the configured options.
     * @return true if connection opened successfully, false otherwise.
     * Use lastError() to get detailed error information on failure.
     */
    [[nodiscard]] bool open() {
        if (m_isOpen) {
            return true;
        }

        if (!m_options.isValid()) {
            m_lastError = m_options.validationError();
            return false;
        }

        // Create database connection with driver
        m_database = QSqlDatabase::addDatabase(m_options.getDriverName(), m_connectionName);

        // Configure connection based on database type
        bool configSuccess = m_options.visit([this](const auto& opts) -> bool {
            using T = std::decay_t<decltype(opts)>;

            if constexpr (std::is_same_v<T, SqliteOptions>) {
                return configureSqlite(opts);
            } else if constexpr (std::is_same_v<T, PostgresOptions>) {
                return configurePostgres(opts);
            } else if constexpr (std::is_same_v<T, MysqlOptions>) {
                return configureMysql(opts);
            }
            return false;
        });

        if (!configSuccess) {
            removeConnection();
            return false;
        }

        // Attempt to open the connection
        if (!m_database.open()) {
            m_lastError = QString("Failed to open database: %1").arg(m_database.lastError().text());
            removeConnection();
            return false;
        }

        m_isOpen = true;
        m_lastError.clear();
        return true;
    }

    /**
     * Closes the database connection and releases resources.
     * Safe to call multiple times or on an already closed connection.
     */
    void close() {
        if (m_isOpen) {
            m_database.close();
            m_isOpen = false;
        }
        removeConnection();
    }

    /**
     * Checks if the connection is currently open.
     * @return true if connection is open and active.
     */
    [[nodiscard]] bool isOpen() const { return m_isOpen && m_database.isOpen(); }

    /**
     * Returns the last error message from connection or query operations.
     * @return Error description, or empty string if no error occurred.
     */
    [[nodiscard]] const QString& lastError() const { return m_lastError; }

    /**
     * Returns the connection name used for this database connection.
     * @return Unique connection identifier.
     */
    [[nodiscard]] const QString& connectionName() const { return m_connectionName; }

    /**
     * Returns the connection options used for this connection.
     * @return Reference to the ConnOptions instance.
     */
    [[nodiscard]] const ConnOptions& options() const { return m_options; }

    /**
     * Returns a reference to the underlying QSqlDatabase object.
     * @return QSqlDatabase instance for direct access to Qt SQL functionality.
     * @note Connection must be open before using the database object.
     */
    [[nodiscard]] QSqlDatabase& database() { return m_database; }

    /**
     * Returns a const reference to the underlying QSqlDatabase object.
     */
    [[nodiscard]] const QSqlDatabase& database() const { return m_database; }

    /**
     * Creates and returns a QSqlQuery associated with this connection.
     * @return QSqlQuery object ready for use.
     * @note Connection must be open before creating queries.
     */
    [[nodiscard]] QSqlQuery createQuery() const { return QSqlQuery(m_database); }

    /**
     * Executes a SQL query and returns the result.
     * @param queryStr SQL query string to execute.
     * @return QSqlQuery with results if successful, invalid query otherwise.
     */
    [[nodiscard]] std::optional<QSqlQuery> executeQuery(const QString& queryStr) {
        if (!isOpen()) {
            m_lastError = "Cannot execute query: connection is not open";
            return std::nullopt;
        }

        QSqlQuery query(m_database);
        if (!query.exec(queryStr)) {
            m_lastError = QString("Query execution failed: %1").arg(query.lastError().text());
            return std::nullopt;
        }

        m_lastError.clear();
        return query;
    }

    /**
     * Executes a SQL statement that doesn't return results (INSERT, UPDATE, DELETE, etc.).
     * @param statement SQL statement to execute.
     * @return true if execution succeeded, false otherwise.
     */
    [[nodiscard]] bool execute(const QString& statement) {
        if (!isOpen()) {
            m_lastError = "Cannot execute statement: connection is not open";
            return false;
        }

        QSqlQuery query(m_database);
        if (!query.exec(statement)) {
            m_lastError = QString("Statement execution failed: %1").arg(query.lastError().text());
            return false;
        }

        m_lastError.clear();
        return true;
    }

    /**
     * Begins a database transaction.
     * @return true if transaction started successfully, false otherwise.
     */
    [[nodiscard]] bool beginTransaction() {
        if (!isOpen()) {
            m_lastError = "Cannot begin transaction: connection is not open";
            return false;
        }

        if (!m_database.transaction()) {
            m_lastError =
                QString("Failed to begin transaction: %1").arg(m_database.lastError().text());
            return false;
        }

        m_lastError.clear();
        return true;
    }

    /**
     * Commits the current transaction.
     * @return true if commit succeeded, false otherwise.
     */
    [[nodiscard]] bool commit() {
        if (!isOpen()) {
            m_lastError = "Cannot commit: connection is not open";
            return false;
        }

        if (!m_database.commit()) {
            m_lastError =
                QString("Failed to commit transaction: %1").arg(m_database.lastError().text());
            return false;
        }

        m_lastError.clear();
        return true;
    }

    /**
     * Rolls back the current transaction.
     * @return true if rollback succeeded, false otherwise.
     */
    [[nodiscard]] bool rollback() {
        if (!isOpen()) {
            m_lastError = "Cannot rollback: connection is not open";
            return false;
        }

        if (!m_database.rollback()) {
            m_lastError =
                QString("Failed to rollback transaction: %1").arg(m_database.lastError().text());
            return false;
        }

        m_lastError.clear();
        return true;
    }

    /**
     * Tests the connection by executing a simple query.
     * @return true if connection is working, false otherwise.
     */
    [[nodiscard]] bool testConnection() {
        if (!isOpen()) {
            m_lastError = "Cannot test connection: connection is not open";
            return false;
        }

        QSqlQuery query(m_database);
        if (!query.exec("SELECT 1")) {
            m_lastError = QString("Connection test failed: %1").arg(query.lastError().text());
            return false;
        }

        m_lastError.clear();
        return true;
    }

    /**
     * Returns the list of available tables in the database.
     * @return List of table names, or empty list on error.
     */
    [[nodiscard]] QStringList tables() const {
        if (!isOpen()) {
            return {};
        }
        return m_database.tables();
    }

   private:
    /**
     * Generates a unique connection name using UUID.
     * @return Unique connection identifier string.
     */
    [[nodiscard]] static QString generateConnectionName() {
        return QString("conn_%1").arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    }

    /**
     * Configures the database connection for SQLite.
     * @param opts SQLite connection options.
     * @return true if configuration succeeded.
     */
    [[nodiscard]] bool configureSqlite(const SqliteOptions& opts) {
        m_database.setDatabaseName(opts.getDbName());
        return true;
    }

    /**
     * Configures the database connection for PostgreSQL.
     * @param opts PostgreSQL connection options.
     * @return true if configuration succeeded.
     */
    [[nodiscard]] bool configurePostgres(const PostgresOptions& opts) {
        m_database.setHostName(opts.getHost());
        m_database.setPort(opts.getPort());
        m_database.setDatabaseName(opts.getDbName());
        m_database.setUserName(opts.getUser());
        m_database.setPassword(opts.getPassword());
        return true;
    }

    /**
     * Configures the database connection for MySQL.
     * @param opts MySQL connection options.
     * @return true if configuration succeeded.
     */
    [[nodiscard]] bool configureMysql(const MysqlOptions& opts) {
        m_database.setHostName(opts.getHost());
        m_database.setPort(opts.getPort());
        m_database.setDatabaseName(opts.getDbName());
        m_database.setUserName(opts.getUser());
        m_database.setPassword(opts.getPassword());
        return true;
    }

    /**
     * Removes the connection from Qt's connection pool.
     * Safe to call even if connection was never added.
     */
    void removeConnection() {
        if (!m_connectionName.isEmpty()) {
            QSqlDatabase::removeDatabase(m_connectionName);
        }
    }

    ConnOptions m_options;        // Connection configuration options
    QString m_connectionName;     // Unique connection identifier
    QSqlDatabase m_database;      // Qt SQL database handle
    bool m_isOpen{};              // Connection state flag
    mutable QString m_lastError;  // Last error message
};

/**
 * RAII transaction guard for automatic transaction management.
 * Commits on successful completion, rolls back on destruction if not committed.
 * 
 * Usage:
 * @code
 * {
 *     TransactionGuard guard(connection);
 *     // Perform database operations
 *     connection.execute("INSERT ...");
 *     guard.commit(); // Explicit commit
 * } // Auto rollback if commit not called
 * @endcode
 */
class TransactionGuard {
   public:
    /**
     * Begins a transaction on the provided connection.
     * @param conn Database connection to manage transaction for.
     */
    explicit TransactionGuard(DatabaseConnection& conn)
        : m_connection(conn), m_committed(false), m_active(false) {
        m_active = m_connection.beginTransaction();
    }

    /**
     * Destructor rolls back transaction if not explicitly committed.
     */
    ~TransactionGuard() {
        if (m_active && !m_committed) {
            bool _ = m_connection.rollback();
        }
    }

    // Non-copyable, non-movable for safety
    TransactionGuard(const TransactionGuard&) = delete;
    TransactionGuard& operator=(const TransactionGuard&) = delete;
    TransactionGuard(TransactionGuard&&) = delete;
    TransactionGuard& operator=(TransactionGuard&&) = delete;

    /**
     * Commits the transaction.
     * @return true if commit succeeded, false otherwise.
     */
    [[nodiscard]] bool commit() {
        if (!m_active || m_committed) {
            return false;
        }

        if (m_connection.commit()) {
            m_committed = true;
            return true;
        }
        return false;
    }

    /**
     * Explicitly rolls back the transaction.
     * @return true if rollback succeeded, false otherwise.
     */
    [[nodiscard]] bool rollback() {
        if (!m_active || m_committed) {
            return false;
        }

        if (m_connection.rollback()) {
            m_active = false;
            return true;
        }
        return false;
    }

    /**
     * Checks if the transaction is active (started and not committed/rolled back).
     */
    [[nodiscard]] bool isActive() const { return m_active && !m_committed; }

   private:
    DatabaseConnection& m_connection;  // Reference to managed connection
    bool m_committed{};                // Commit status flag
    bool m_active{};                   // Transaction active flag
};

#endif /* DATABASE_CONNECTION_H */
