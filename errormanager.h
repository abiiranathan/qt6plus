#ifndef ERROR_H
#define ERROR_H

#include <QMutex>
#include <QString>

class ErrorManager {
 public:
  static void setError(const QString& errorMessage);
  static const QString lastError();
  static bool hasError();

 private:
  static QString error;
  static QMutex mutex;
};

#endif  // ERROR_H
