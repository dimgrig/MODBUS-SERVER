/*
    Copyright (c) 2009-10 Qtrac Ltd. All rights reserved.

    This program or module is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation, either version 3 of
    the License, or (at your option) any later version. It is provided
    for educational purposes and is distributed in the hope that it will
    be useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU General Public License for more details.
*/

#include "aqp.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QPushButton>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QRegExp>
#endif
#if QT_VERSION > QT_VERSION_CHECK(5, 0, 0)
#include <QRegularExpression>
#endif
#include <QSet>
#include <QStringList>
#include <QTextStream>
#include <QTimer>

#include "templates.h"
#include "MessageBox.h"

static QHash<QString, QSet<QString> > formatForMimeType;

void readMimeTypeData(const QString &filename)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QRegExp whitespace("\\s+");
#endif
#if QT_VERSION > QT_VERSION_CHECK(5, 15, 8)
    QRegularExpression whitespace("\\s+");
#endif
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly|QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(whitespace);
            if (parts.isEmpty())
                continue;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                QSet<QString> suffixes = QSet<QString>::fromList(parts.mid(1));
#endif
#if QT_VERSION > QT_VERSION_CHECK(5, 15, 8)
                QSet<QString> suffixes = QListToQSet(parts.mid(1));
#endif

            formatForMimeType[parts.at(0)].unite(suffixes);
        }
    }
}


void initializeFormatForMimeType()
{
    if (formatForMimeType.isEmpty()) {
        QString filename("/etc/mime.types");
        if (QFile::exists(filename))
            readMimeTypeData(filename);
        readMimeTypeData(":/mime.types");
    }
}


namespace AQP {

void information(QWidget *parent, const QString &title,
                 const QString &text, const QString &detailedText)
{
//#if QT_VERSION >= 0x040600
//    QScopedPointer<QMessageBox> messageBox(new QMessageBox(parent));
//#else
//    QSharedPointer<QMessageBox> messageBox(new QMessageBox(parent));
//#endif
//    //messageBox->setOption(QMessageBox::Option::DontUseNativeDialog, true);
    MessageBox *messageBox = new MessageBox(parent);
    if (parent)
        messageBox->setWindowModality(Qt::NonModal);
    messageBox->setAttribute(Qt::WA_DeleteOnClose, true);
    messageBox->setWindowTitle(QString("%1 - %2")
            .arg(QApplication::applicationName()).arg(title));
    messageBox->setText(text);
    if (!detailedText.isEmpty())
        messageBox->setInformativeText(detailedText);
    messageBox->setIcon(QMessageBox::Information);
    messageBox->addButton(QMessageBox::Ok);
    messageBox->show();
}

void error(QWidget *parent, const QString &title,
             const QString &text, const QString &detailedText) {
#if QT_VERSION >= 0x040600
    QScopedPointer<MessageBox> messageBox(new MessageBox(parent));
#else
    QSharedPointer<MessageBox> messageBox(new MessageBox(parent));
#endif
    if (parent)
        messageBox->setWindowModality(Qt::WindowModal);
    else
        messageBox->setWindowModality(Qt::NonModal);
    messageBox->setWindowModality(Qt::NonModal);
    messageBox->setAttribute(Qt::WA_DeleteOnClose, true);
    messageBox->setWindowTitle(QString("%1 - %2")
                                   .arg(QApplication::applicationName()).arg(title));
    messageBox->setText(text);
    if (!detailedText.isEmpty())
        messageBox->setInformativeText(detailedText);
    messageBox->setIcon(QMessageBox::Warning);
    messageBox->addButton(QMessageBox::Ok);
    messageBox->exec();
}

void warning(QWidget *parent, const QString &title,
             const QString &text, const QString &detailedText, int delay, int close) {
//#if QT_VERSION >= 0x040600
//    QScopedPointer<MessageBox> messageBox(new MessageBox(parent));
//#else
//    QSharedPointer<MessageBox> messageBox(new MessageBox(parent));
//#endif
//    if (parent)
//        messageBox->setWindowModality(Qt::WindowModal);
//    else
//        messageBox->setWindowModality(Qt::NonModal);
    MessageBox *messageBox = new MessageBox(parent);
    messageBox->setWindowModality(Qt::NonModal);
    messageBox->setAttribute(Qt::WA_DeleteOnClose, true);
    messageBox->setWindowTitle(QString("%1 - %2")
                                   .arg(QApplication::applicationName()).arg(title));
    messageBox->setText(text);
    if (!detailedText.isEmpty())
        messageBox->setInformativeText(detailedText);
    messageBox->setIcon(QMessageBox::Warning);
    messageBox->addButton(QMessageBox::Ok);

    messageBox->set_delay(delay);
    messageBox->set_close(close);
    messageBox->show();
}

bool question(QWidget *parent, const QString &title,
              const QString &text, const QString &detailedText,
              const QString &yesText, const QString &noText)
{
#if QT_VERSION >= 0x040600
    QScopedPointer<QMessageBox> messageBox(new QMessageBox(parent));
#else
    QSharedPointer<QMessageBox> messageBox(new QMessageBox(parent));
#endif
    if (parent)
        messageBox->setWindowModality(Qt::WindowModal);
    messageBox->setWindowTitle(QString("%1 - %2")
            .arg(QApplication::applicationName()).arg(title));
    messageBox->setText(text);
    if (!detailedText.isEmpty())
        messageBox->setInformativeText(detailedText);
    messageBox->setIcon(QMessageBox::Question);
    QAbstractButton *yesButton = messageBox->addButton(yesText,
            QMessageBox::AcceptRole);
    messageBox->addButton(noText, QMessageBox::RejectRole);
    messageBox->setDefaultButton(
            qobject_cast<QPushButton*>(yesButton));
    messageBox->exec();
    return messageBox->clickedButton() == yesButton;
}


bool okToDelete(QWidget *parent, const QString &title,
                const QString &text, const QString &detailedText)
{
#if QT_VERSION >= 0x040600
    QScopedPointer<QMessageBox> messageBox(new QMessageBox(parent));
#else
    QSharedPointer<QMessageBox> messageBox(new QMessageBox(parent));
#endif
    if (parent)
        messageBox->setWindowModality(Qt::WindowModal);
    messageBox->setIcon(QMessageBox::Question);
    messageBox->setWindowTitle(QString("%1 - %2")
            .arg(QApplication::applicationName()).arg(title));
    messageBox->setText(text);
    if (!detailedText.isEmpty())
        messageBox->setInformativeText(detailedText);
    QAbstractButton *deleteButton = messageBox->addButton(
            QObject::tr("У&далить"), QMessageBox::AcceptRole);
    messageBox->addButton(QObject::tr("Отме&на"),
                          QMessageBox::RejectRole);
    messageBox->setDefaultButton(
            qobject_cast<QPushButton*>(deleteButton));
    messageBox->exec();
    return messageBox->clickedButton() == deleteButton;
}


void hoursMinutesSecondsForMSec(const int msec, int *hours,
                                int *minutes, int *seconds)
{
    if (hours)
        *hours = (msec / (60 * 60 * 1000)) % 60;
    if (minutes)
        *minutes = (msec / (60 * 1000)) % 60;
    if (seconds)
        *seconds = (msec / 1000) % 60;
}


QString applicationPathOf(const QString &path)
{
    QDir dir(QApplication::applicationDirPath());
#ifdef Q_WS_WIN
    if (dir.dirName().toLower() == "debug" ||
        dir.dirName().toLower() == "release")
        dir.cdUp();
#elif defined(Q_WS_MAC)
    if (dir.dirName() == "MacOS") {
        dir.cdUp();
        dir.cdUp();
        dir.cdUp();
    }
#endif
    if (!path.isEmpty())
        dir.cd(path);
    return dir.canonicalPath();
}


QString filenameFilter(const QString &name,
                       const QList<QByteArray> formats)
{
    QStringList fileFormatList;
    foreach (const QByteArray &ba, formats)
        fileFormatList += ba;
    if (fileFormatList.isEmpty())
        fileFormatList << "*";
    else
        fileFormatList.sort();
    QString fileFormats = QString("%1 (*.").arg(name);
    fileFormats += fileFormatList.join(" *.") + ")";
    return fileFormats;
}


QString filenameFilter(const QString &name,
                       const QStringList &mimeTypes)
{
    initializeFormatForMimeType();
    QSet<QString> formats;
    foreach (const QString &mimeType, mimeTypes)
        formats.unite(formatForMimeType.value(mimeType));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QStringList fileFormatList = QList<QString>::fromSet(formats);
#endif
#if QT_VERSION > QT_VERSION_CHECK(5, 15, 8)
    QStringList fileFormatList = QSetToQStringList(formats);
#endif
    if (fileFormatList.isEmpty())
        fileFormatList << "*";
    else
        fileFormatList.sort();
    QString fileFormats = QString("%1 (*.").arg(name);
    fileFormats += fileFormatList.join(" *.") + ")";
    return fileFormats;
}


QSet<QString> suffixesForMimeTypes(const QStringList &mimeTypes)
{
    initializeFormatForMimeType();
    QSet<QString> formats;
    foreach (const QString &mimeType, mimeTypes)
        formats.unite(formatForMimeType.value(mimeType));
    if (mimeTypes.count() && mimeTypes.at(0).startsWith("audio/")) {
        // TWEAK: Supported audio formats that are not reported as such:
        formats.insert("wav");
    }
    return formats;
}


QVector<int> chunkSizes(const int size, const int chunkCount)
{
    Q_ASSERT(size > 0 && chunkCount > 0);
    if (chunkCount == 1)
        return QVector<int>() << size;
    QVector<int> result(chunkCount, size / chunkCount);
    if (int remainder = size % chunkCount) {
        int index = 0;
        for (int i = 0; i < remainder; ++i) {
            ++result[index];
            ++index;
            index %= chunkCount;
        }
    }
    return result;
}


} // namespace AQP
