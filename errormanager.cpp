#include "errormanager.h"

QString ErrorManager::error;
QMutex ErrorManager::mutex;

void ErrorManager::setError(const QString& errorMessage) {
    QMutexLocker locker(&mutex);
    error = errorMessage;
}

const QString ErrorManager::lastError() {
    QMutexLocker locker(&mutex);
    QString errorMessage = error;
    error.clear();
    return errorMessage;
}

bool ErrorManager::hasError() {
    return !error.isEmpty();
}
