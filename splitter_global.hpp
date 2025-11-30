#ifndef SPLITTER_GLOBAL_H
#define SPLITTER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SPLITTER_LIBRARY)
#  define SPLITTER_EXPORT Q_DECL_EXPORT
#else
#  define SPLITTER_EXPORT Q_DECL_IMPORT
#endif

#endif // SPLITTER_GLOBAL_H
