#include <QScreen>
#include <QMessageBox>
#include <QJsonDocument>
#include <QClipboard>

#include "utils.h"
#include "lib/config.h"

bool Utils::readJsonFile(QIODevice &device, QSettings::SettingsMap &map) {
  QJsonDocument json = QJsonDocument::fromJson(device.readAll());
  map = json.object().toVariantMap();
  return true;
}

bool Utils::writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map) {
  device.write(QJsonDocument(QJsonObject::fromVariantMap(map)).toJson(QJsonDocument::Indented));
  return true;
}

bool Utils::fileExists(const QString &path) {
  QFileInfo check_file(path);
  return check_file.exists() && check_file.isFile();
}

bool Utils::dirExists(const QString &path) {
  QDir pathDir(path);
  return pathDir.exists();
}

QByteArray Utils::fileOpen(const QString &path) {
  QFile file(path);
  if(!file.open(QFile::ReadOnly | QFile::Text)) {
    return QByteArray();
  }

  QByteArray data = file.readAll();
  file.close();
  return data;
}

QByteArray Utils::fileOpenQRC(const QString &path) {
  QFile file(path);
  if(!file.open(QIODevice::ReadOnly)) {
    qDebug() << "error: " << file.errorString();
  }

  QByteArray data = file.readAll();
  file.close();
  return data;
}

bool Utils::isCyrillic(const QString &inp) {
  QRegularExpression re("[А-Яа-яЁё]+");
  QRegularExpressionMatch match = re.match(inp);
  return match.hasMatch();
}

QString Utils::transliteration(const QString &lang, const QString &inp) {
  auto data = Utils::fileOpenQRC(":/assets/cyrillic-transliteration.json");
  auto doc = QJsonDocument::fromJson(data);
  auto obj = doc.object();
  if(!obj.contains(lang)) {
    qDebug() << "could not find " << lang << " in transliteration db";
    return "";
  }

  auto obj_lang = doc[lang].toObject();
  auto obj_lang_to_latin = obj_lang["tolatin"].toObject();
  QString transliteration;

  for(auto chr: inp) {
    if(obj_lang_to_latin.contains(chr)) {
      transliteration += obj_lang_to_latin[chr].toString();
    } else {
      transliteration += chr;
    }
  }

  return transliteration;
}