#ifndef DATABASE_OPTIONS_H
#define DATABASE_OPTIONS_H

#include <QDebug>
#include <QString>
#include <type_traits>
#include <variant>

#include "qt6plus_export.hpp"

/** Enumeration of supported database drivers. */
enum class QT6PLUS_EXPORT Driver : uint8_t {
    SQLITE,    // SQLite database
    POSTGRES,  // PostgreSQL database
    MYSQL      // MySQL/MariaDB database
};

/** SQLite database connection options. */
struct QT6PLUS_EXPORT SqliteOptions {
    /** Database file name or path. */
    QString dbName = "db.sqlite3";

    SqliteOptions() = default;
    explicit SqliteOptions(QString name) : dbName(std::move(name)) {}

    /** Returns the database name (for interface consistency). */
    [[nodiscard]] const QString& getDbName() const { return dbName; }

    /** Returns the connection string (database file path for SQLite). */
    [[nodiscard]] QString getConnectionString() const { return dbName; }

    /** Validates that the database name is not empty. */
    [[nodiscard]] bool isValid() const { return !dbName.isEmpty(); }

    /** Returns validation error message, or empty string if valid. */
    [[nodiscard]] QString validationError() const {
        if (dbName.isEmpty()) {
            return "Database name cannot be empty";
        }
        return {};
    }

    bool operator==(const SqliteOptions& other) const { return dbName == other.dbName; }
};

// Tag types for templated database options
struct PostgresTag {};
struct MysqlTag {};

/**
 * Templated database connection options for PostgreSQL and MySQL.
 * @tparam DatabaseTag Type tag (PostgresTag or MysqlTag) to distinguish database types.
 */
template <typename DatabaseTag>
class QT6PLUS_EXPORT DatabaseOptions {
   public:
    /** Builder class for fluent construction of DatabaseOptions. */
    class Builder {
       public:
        Builder& dbName(QString name) {
            m_dbname = std::move(name);
            return *this;
        }
        Builder& user(QString u) {
            m_user = std::move(u);
            return *this;
        }
        Builder& password(QString pwd) {
            m_password = std::move(pwd);
            return *this;
        }
        Builder& host(QString h) {
            m_host = std::move(h);
            return *this;
        }
        Builder& port(int p) {
            m_port = p;
            return *this;
        }

        /** Builds the DatabaseOptions instance with configured values. */
        [[nodiscard]] DatabaseOptions build() const {
            return DatabaseOptions(m_dbname, m_user, m_password,
                                   m_host.isEmpty() ? QString("127.0.0.1") : m_host,
                                   m_port == 0 ? DatabaseOptions::defaultPort() : m_port);
        }

       private:
        QString m_dbname;
        QString m_user;
        QString m_password;
        QString m_host;
        int m_port = 0;
    };

    DatabaseOptions() = default;

    /**
     * Constructs database options with all connection parameters.
     * @param dbName Database name.
     * @param user Username for authentication.
     * @param password Password for authentication.
     * @param host Database server hostname or IP address.
     * @param port Database server port number.
     */
    DatabaseOptions(QString dbName, QString user, QString password, QString host, int port)
        : m_dbname(std::move(dbName)),
          m_user(std::move(user)),
          m_password(std::move(password)),
          m_host(std::move(host)),
          m_port(port) {}

    /** Returns the database name. */
    [[nodiscard]] const QString& getDbName() const { return m_dbname; }

    /** Returns the username. */
    [[nodiscard]] const QString& getUser() const { return m_user; }

    /** Returns the password. */
    [[nodiscard]] const QString& getPassword() const { return m_password; }

    /** Returns the host address. */
    [[nodiscard]] const QString& getHost() const { return m_host; }

    /** Returns the port number. */
    [[nodiscard]] int getPort() const { return m_port; }

    /**
     * Returns the default port for the database type.
     * @return 5432 for PostgreSQL, 3306 for MySQL, 0 otherwise.
     */
    [[nodiscard]] static constexpr int defaultPort() noexcept {
        if constexpr (std::is_same_v<DatabaseTag, PostgresTag>) {
            return 5432;
        } else if constexpr (std::is_same_v<DatabaseTag, MysqlTag>) {
            return 3306;
        }
        return 0;
    }

    /**
     * Validates the connection options.
     * Checks that required fields are not empty and port is in valid range (1-65535).
     */
    [[nodiscard]] bool isValid() const {
        if (m_port <= 0 || m_port > 65535) {
            return false;
        }
        return !m_dbname.isEmpty() && !m_user.isEmpty() && !m_host.isEmpty();
    }

    /**
     * Returns a detailed validation error message.
     * @return Error description, or empty string if valid.
     */
    [[nodiscard]] QString validationError() const {
        if (m_dbname.isEmpty()) {
            return "Database name cannot be empty";
        }
        if (m_user.isEmpty()) {
            return "User cannot be empty";
        }
        if (m_host.isEmpty()) {
            return "Host cannot be empty";
        }
        if (m_port <= 0 || m_port > 65535) {
            return "Port must be between 1 and 65535";
        }
        return {};
    }

    /**
     * Generates a connection string suitable for the database driver.
     * @return Formatted connection string with all parameters.
     */
    [[nodiscard]] QString getConnectionString() const {
        return QString("host=%1 port=%2 dbname=%3 user=%4 password=%5")
            .arg(m_host)
            .arg(m_port)
            .arg(m_dbname)
            .arg(m_user)
            .arg(m_password);
    }

    /** Creates a new Builder instance for fluent construction. */
    [[nodiscard]] static Builder builder() { return Builder{}; }

    bool operator==(const DatabaseOptions& other) const {
        return m_dbname == other.m_dbname && m_user == other.m_user &&
               m_password == other.m_password && m_host == other.m_host && m_port == other.m_port;
    }

   private:
    QString m_dbname;
    QString m_user;
    QString m_password;
    QString m_host = "127.0.0.1";
    int m_port = defaultPort();
};

/** Type alias for PostgreSQL connection options. */
using QT6PLUS_EXPORT PostgresOptions = DatabaseOptions<PostgresTag>;

/** Type alias for MySQL connection options. */
using QT6PLUS_EXPORT MysqlOptions = DatabaseOptions<MysqlTag>;

/**
 * Concept constraining template parameter to valid database option types.
 * @tparam T Type to validate.
 */
template <typename T>
concept QT6PLUS_EXPORT DatabaseOptionType =
    std::is_same_v<T, SqliteOptions> || std::is_same_v<T, PostgresOptions> ||
    std::is_same_v<T, MysqlOptions>;

/**
 * Unified connection options class supporting multiple database types.
 * Uses std::variant to hold one of the supported database option types.
 */
class QT6PLUS_EXPORT ConnOptions {
   public:
    /** Variant type holding any of the supported database options. */
    using Variant = std::variant<SqliteOptions, PostgresOptions, MysqlOptions>;

    /** Default constructor creates SQLite options. */
    ConnOptions() : m_options(SqliteOptions()) {}

    /** Constructs with SQLite options. */
    explicit ConnOptions(SqliteOptions opt) : m_options(std::move(opt)) {}

    /** Constructs with PostgreSQL options. */
    explicit ConnOptions(PostgresOptions opt) : m_options(std::move(opt)) {}

    /** Constructs with MySQL options. */
    explicit ConnOptions(MysqlOptions opt) : m_options(std::move(opt)) {}

    /**
     * Returns the driver type based on the held variant type.
     * Derived from variant index, no redundant storage.
     */
    [[nodiscard]] Driver getDriver() const { return static_cast<Driver>(m_options.index()); }

    /**
     * Returns the Qt SQL driver name string.
     * @return "QSQLITE", "QPSQL", or "QMYSQL".
     */
    [[nodiscard]] QString getDriverName() const {
        return std::visit(
            [](const auto& opt) -> QString {
                using T = std::decay_t<decltype(opt)>;
                if constexpr (std::is_same_v<T, SqliteOptions>) {
                    return "QSQLITE";
                } else if constexpr (std::is_same_v<T, PostgresOptions>) {
                    return "QPSQL";
                } else if constexpr (std::is_same_v<T, MysqlOptions>) {
                    return "QMYSQL";
                }
            },
            m_options);
    }

    /**
     * Returns the connection string for the held database options.
     * Format varies by database type.
     */
    [[nodiscard]] QString getConnectionString() const {
        return std::visit([](const auto& opt) { return opt.getConnectionString(); }, m_options);
    }

    /**
     * Validates the held database options.
     * @return true if the options are valid for their database type.
     */
    [[nodiscard]] bool isValid() const {
        return std::visit([](const auto& opt) { return opt.isValid(); }, m_options);
    }

    /**
     * Returns detailed validation error from the held options.
     * @return Error description, or empty string if valid.
     */
    [[nodiscard]] QString validationError() const {
        return std::visit([](const auto& opt) { return opt.validationError(); }, m_options);
    }

    /** Checks if holding SQLite options. */
    [[nodiscard]] bool isSqlite() const { return std::holds_alternative<SqliteOptions>(m_options); }

    /** Checks if holding PostgreSQL options. */
    [[nodiscard]] bool isPostgres() const {
        return std::holds_alternative<PostgresOptions>(m_options);
    }

    /** Checks if holding MySQL options. */
    [[nodiscard]] bool isMysql() const { return std::holds_alternative<MysqlOptions>(m_options); }

    /**
     * Returns a reference to the held options of the specified type.
     * @tparam T Database option type (must be SqliteOptions, PostgresOptions, or MysqlOptions).
     * @throws std::bad_variant_access if the held type does not match T.
     */
    template <typename T>
    [[nodiscard]] const T& get() const requires DatabaseOptionType<T> {
        return std::get<T>(m_options);
    }

    /**
     * Returns a pointer to the held options if they match the specified type.
     * @tparam T Database option type to check for.
     * @return Pointer to options if type matches, nullptr otherwise.
     */
    template <typename T>
    [[nodiscard]] const T* getIf() const noexcept requires DatabaseOptionType<T> {
        return std::get_if<T>(&m_options);
    }

    /**
     * Applies a visitor to the held database options.
     * @tparam Visitor Callable type that accepts any database option type.
     * @param visitor Visitor function to apply.
     * @return Result of visitor invocation.
     */
    template <typename Visitor>
    decltype(auto) visit(Visitor&& visitor) const {
        return std::visit(std::forward<Visitor>(visitor), m_options);
    }

    bool operator==(const ConnOptions& other) const { return m_options == other.m_options; }

    /** QDebug stream operator for debugging output. */
    friend QDebug operator<<(QDebug debug, const ConnOptions& opts) {
        QDebugStateSaver saver(debug);
        debug.nospace() << "ConnOptions(" << opts.getDriverName() << ", valid=" << opts.isValid()
                        << ")";
        return debug;
    }

   private:
    Variant m_options;
};

#endif /* DATABASE_OPTIONS_H */
