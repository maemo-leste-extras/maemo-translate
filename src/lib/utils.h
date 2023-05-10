#pragma once

#include <QObject>
#include <QSettings>
#include <QRegExp>
#include <QTextCodec>
#include <QStandardItemModel>
#include <QApplication>
#include <QTextCharFormat>

class Utils
{

public:
    static bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map);
    static bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map);
    static bool fileExists(const QString &path);
    static QByteArray fileOpen(const QString &path);
    static QByteArray fileOpenQRC(const QString &path);
    static bool isCyrillic(const QString &inp);
    static QString transliteration(const QString &lang, const QString &inp);
    static QString barrayToString(const QByteArray &data);
    static bool dirExists(const QString &path);
};
