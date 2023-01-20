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
