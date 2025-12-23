#include <QApplication>
#include <QStyleFactory>
#include <QProcess>
#include <QSplashScreen>
#include <QTranslator>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QDesktopWidget>
#endif

#include "aqp.h"

#include "Config.h"
#include "Logger.h"

#include "icons.h"
#include "mainwindow.h"

//#include "Modules/AUTH/Auth.h"
//#include "Modules/AUTH/User.h"

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

//void bash() {
//    QProcess process1;
//    process1.start("sudo su");

//    process1.start("chmod 777 /dev/ptyp98");
//    process1.start("chmod 777 /dev/ptyp99");
//    process1.start("rm /tmp/RMO_diode_local_server");
//}

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
    Application app(argc, argv);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    QSplashScreen splash;
    int splash_size = 200;
    splash.setGeometry(0, 0, splash_size, splash_size);
    splash.setPixmap(SPLASH.scaled(splash_size, splash_size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

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

    app.setWindowIcon(ICON);

    QTranslator translator;
    //C:\Qt\5.15.2\mingw81_32\translations
    translator.load(QTBASE_RU);
    //translator.load(QString("QtLanguage_") + QString("ru_RU"));
    app.installTranslator(&translator);

    //fix locale in debug
    //https://ru.stackoverflow.com/questions/723433/qt5-%D0%BE%D1%82%D0%BB%D0%B0%D0%B4%D1%87%D0%B8%D0%BA-%D0%B2%D1%8B%D0%B4%D0%B0%D1%91%D1%82-%D0%BD%D0%B5%D1%87%D0%B8%D1%82%D0%B0%D0%B5%D0%BC%D1%8B%D0%B9-%D1%82%D0%B5%D0%BA%D1%81%D1%82
    //sudo nano /usr/share/applications/org.qt-project.qtcreator.desktop
    //Exec=env LC_ALL=ru_RU.UTF-8 qtcreator %F

//    QString locale = QLocale::system().name(); // запрос языка (например: "ru")
//    QTranslator* translator = new QTranslator; // создаем переводчик
//    translator->load(QString("qt_") + locale);  // загрузка файла qt_*.qm*
//    app.installTranslator(translator);         // устанока переводчика

//    signal(SIGSEGV, handler_sigsegv);
//    signal(SIGFPE, handler_sigfpe);

    MainWindow w;
    //splash.close();
    w.show();    
    splash.finish(&w);
    //w.init_auth_users();

    //qCDebug(logInfo()) << "RMO_diode started";
    QApplication::restoreOverrideCursor();
    return app.exec();    
}
