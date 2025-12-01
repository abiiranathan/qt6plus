#ifndef QT6PLUS_EXPORT_HPP
#define QT6PLUS_EXPORT_HPP

#ifdef QT6PLUS_STATIC_DEFINE
// Building or using static library - no export/import needed
#define QT6PLUS_EXPORT
#define QT6PLUS_NO_EXPORT
#else
#ifndef QT6PLUS_EXPORT
#ifdef _WIN32
// Windows DLL export/import
#ifdef qt6plus_EXPORTS
// Building the DLL
#define QT6PLUS_EXPORT __declspec(dllexport)
#else
// Using the DLL
#define QT6PLUS_EXPORT __declspec(dllimport)
#endif
#elif defined(__GNUC__) || defined(__clang__)
// GCC/Clang - use visibility attributes
#ifdef qt6plus_EXPORTS
#define QT6PLUS_EXPORT __attribute__((visibility("default")))
#else
#define QT6PLUS_EXPORT
#endif
#else
// Unknown compiler - no export
#define QT6PLUS_EXPORT
#endif
#endif

#ifndef QT6PLUS_NO_EXPORT
#ifdef _WIN32
#define QT6PLUS_NO_EXPORT
#elif defined(__GNUC__) || defined(__clang__)
#define QT6PLUS_NO_EXPORT __attribute__((visibility("hidden")))
#else
#define QT6PLUS_NO_EXPORT
#endif
#endif
#endif

// Deprecated macro (optional)
#ifndef QT6PLUS_DEPRECATED
#ifdef _WIN32
#define QT6PLUS_DEPRECATED __declspec(deprecated)
#elif defined(__GNUC__) || defined(__clang__)
#define QT6PLUS_DEPRECATED __attribute__((__deprecated__))
#else
#define QT6PLUS_DEPRECATED
#endif
#endif

#endif  // QT6PLUS_EXPORT_HPP
