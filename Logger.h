#ifndef ASTRA_LOGGER_H
#define ASTRA_LOGGER_H

#include <iostream>

#include <QCoreApplication>
#include <QFile>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QLoggingCategory>
#include <QTextStream>

Q_DECLARE_LOGGING_CATEGORY(logDebug)
Q_DECLARE_LOGGING_CATEGORY(logInfo)
Q_DECLARE_LOGGING_CATEGORY(logWarning)
Q_DECLARE_LOGGING_CATEGORY(logCritical)
Q_DECLARE_LOGGING_CATEGORY(logFatal)

#define LOG_DT_FORMAT "yyyy-MM-dd HH:mm:ss.zzz"

enum class LOG_LEV {
    DEBUG,
    INFO,
    WARNING,
    CRITICAL,
    FATAL,
    NONE_LOG
};

class Logger {
public:
    static void log_init();
    static void log_delete();
    static void LogMessageOutput([[maybe_unused]] QtMsgType type, const QMessageLogContext &, const QString & str);
    static void LogFileOpening(int change);

    static bool __log_enable;
    static bool __log_to_file;
    static bool __log_date;
    static std::string __log_file;
    static int __log_keep_days;
    static LOG_LEV __log_level;
};

//#define LOG_STDOUT

//#ifdef LOG_STDOUT
//    #define LOG(...) std::cout << __VA_ARGS__ << std::endl
//#elif LOG_PRINTF
//    #define LOG(...) printf(__VA_ARGS__)
//#elif defined LOG_QT
//    #define LOG(...)
//#else
//    #define LOG(...)
//#endif
#endif //ASTRA_LOGGER_H
