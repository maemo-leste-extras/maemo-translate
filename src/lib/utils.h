#pragma once

#include <QObject>
#include <QSettings>
#include <QRegExp>
#include <QStandardItemModel>
#include <QApplication>
#include <QTextCharFormat>

class Utils
{

public:
    static bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map);
    static bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map);
};

