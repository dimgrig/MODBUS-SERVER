#include <QApplication>
#include <QStyleFactory>
#include <QProcess>
#include <QResource>
#include <QScreen>
#include <QSplashScreen>
#include <QStandardPaths>
#include <QTranslator>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QDesktopWidget>
#endif

#include <core-widgets-lib/aqp.h>
#include <core-journal-lib/Categories.h>
#include <core-widgets-lib/IconManager.h>
#include <core-journal-lib/JournalAccess.h>
#include <core-lib/Logger.h>
#include <core-widgets-lib/ConnectionButton.h>
#include <core-modbus-lib/ModbusSettings.h>

#include "Config.h"
#include "mainwindow.h"

#include <signal.h>

class Application final : public QApplication {
public:
    Application(int& argc, char** argv) : QApplication(argc, argv) {}
    virtual bool notify(QObject *receiver, QEvent *event) override {
        try {
            return QApplication::notify(receiver, event);
        } catch (AQP::Error &e) {
//            qCDebug(logFatal(), "Error %s sending event %s to object %s (%s)",
//                e.what(), typeid(e).name(), qPrintable(receiver->objectName()),
//                typeid(*receiver).name());
        } catch (...) {
            qCDebug(logFatal(), "Error <unknown> sending event %s to object %s (%s)",
                typeid(event).name(), qPrintable(receiver->objectName()),
                typeid(*receiver).name());
        }

        // qFatal aborts, so this isn't really necessary
        // but you might continue if you use a different logging lib
        return false;
    }
};

void handler_sigsegv(int signum) {
    //MessageBoxA(NULL, "SIGSEGV Error!", "POSIX Signal", MB_ICONSTOP);
    signal(signum, SIG_DFL);
    exit(3);
}

void handler_sigfpe(int signum) {
    //MessageBoxA(NULL, "SIGFPE Error!", "POSIX Signal", MB_ICONSTOP);
    signal(signum, SIG_DFL);
    exit(3);
}

int main(int argc, char *argv[]) {
    {
        QCoreApplication::setApplicationName("modbus-server"); // важно для AppConfigLocation
        QCoreApplication::setOrganizationName("ACS");

#ifdef Q_OS_WIN
        ;
#else
        CoreConfig::setUserDataDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
#endif
    }

    Application app(argc, argv);
    Q_INIT_RESOURCE(resources);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QSplashScreen splash;
    int splash_size = 200;
    splash.setGeometry(0, 0, splash_size, splash_size);
    splash.setPixmap(IconManager::getPixmap(":/icons/splash_modbus_server.svg", QSize(splash_size, splash_size)));

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    splash.move(QApplication::desktop()->screen()->rect().center() - splash.rect().center());
#endif
#if QT_VERSION > QT_VERSION_CHECK(5, 15, 8)
    splash.move(QApplication::primaryScreen()->geometry().center() - splash.rect().center());
#endif
    splash.show();
    app.processEvents();

    volatile bool log = cfg.LOG; //calls log_init();
    Application::setStyle(QStyleFactory::create("qt5ct-style"));
    //qDebug(logInfo()) << QStyleFactory::keys();
    app.setApplicationDisplayName(cfg.APPLICATION_NAME);

    // Identity приложения — из локального qrc
    IconManager::setApplicationIcon(QIcon(":/icons/modbus.ico"));
    IconManager::setApplicationLogo(QPixmap(":/icons/modbus.svg"));
    app.setWindowIcon(IconManager::applicationIcon());

    MainWindow w;
    //splash.close();
    w.show();    
    splash.finish(&w);
    //w.init_auth_users();

    // ── Автоподключение (удобно для CI/диагностики) ──────────
    if (qEnvironmentVariableIsSet("MODBUS_AUTOCONNECT")) {
        qInfo() << "MODBUS_AUTOCONNECT=1 — triggering connect after 200ms";
        QTimer::singleShot(200, &w, [&w]() {
            if (auto* btn = w.findChild<ConnectionButton*>("connBtn")) {
                emit btn->connectRequested();
                qCDebug(logDebug) << "MODBUS_AUTOCONNECT: connectRequested emitted";
            } else {
                qCDebug(logDebug) << "MODBUS_AUTOCONNECT: ConnectionButton not found";
            }
        });
    }

    // ── Smoke / CI-режим ─────────────────────────────────────
    // MODBUS_SMOKE=1 — приложение стартует, через 1000 мс проверяет
    // ключевые элементы UI и выходит с кодом 0/1.
    if (qEnvironmentVariableIsSet("MODBUS_SMOKE")) {
        QTimer::singleShot(1000, &w, [&w]() {
            int failures = 0;

            auto* btn = w.findChild<ConnectionButton*>("connBtn");
            if (btn) {
                qCDebug(logDebug).noquote()
                << "[ OK ] ConnectionButton found, text=" << btn->text()
                << " iconNull=" << btn->icon().isNull();
            } else {
                qCDebug(logDebug) << "[FAIL] ConnectionButton 'connBtn' not found";
                ++failures;
            }

            auto* settings = w.findChild<QWidget*>("modbusSettingsUI");
            if (settings) {
                qCDebug(logDebug).noquote()
                << "[ OK ] ModbusSettingsUI found, enabled=" << settings->isEnabled();
            } else {
                qCDebug(logDebug) << "[FAIL] modbusSettingsUI not found";
                ++failures;
            }

            qCDebug(logDebug) << "SMOKE FAILURES:" << failures;
            QCoreApplication::exit(failures > 0 ? 1 : 0);
        });
    }

    QApplication::restoreOverrideCursor();
    return app.exec();    
}
