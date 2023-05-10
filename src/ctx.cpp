#include <filesystem>

#include <QObject>
#include <QDir>
#include <QStandardPaths>

#include "ctx.h"
#include "lib/utils.h"

using namespace std::chrono;

void TranslationThread::run() {
  while(true) {  // fetch from task queue, run forever
    TranslationTask task = tasks->get();
    const auto message = task.source;
    if (message.isEmpty())
        continue;

    emit translationStarted();
    milliseconds then = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

    task.result = QString::fromStdString(task.kotki->translate(
      message.toStdString(),
      task.model.toStdString()));

    milliseconds now = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
    auto took = duration_cast<milliseconds>(now - then);
    task.timing = took.count();

    task.err = task.result.isEmpty();
    emit translationEnded(task);
  }
}


AppContext::AppContext() {
  configRoot = QDir::homePath();
  homeDir = QDir::homePath();
  configDirectory = QString("%1/.config/%2/").arg(configRoot, QCoreApplication::applicationName());
  createConfigDirectory(configDirectory);

  kotki = new Kotki();
  kotki->scan();

  for (const auto &pair: kotki->listModels())
    kotkiModels << QString::fromStdString(pair.first);

  // setup translation thread
  m_tasks = QSharedPointer<MessageQueue>::create();
  translationThread = new TranslationThread(m_tasks);
  connect(translationThread, &TranslationThread::translationStarted, this, &AppContext::translationStarted);
  connect(translationThread, &TranslationThread::translationEnded, this, &AppContext::translationEnded);
  connect(translationThread, &TranslationThread::translationEnded, this, &AppContext::onTranslationEnded);
  translationThread->start();

  // preload
  TranslationTask task;
  task.source = "Hello world";
  task.kotki = kotki;
  task.hidden = false;
  task.popularity = false;
  task.model = "ende";

  // preload most popular language model
  auto popularityContestStr = config()->get(ConfigKeys::PopularityContest).toByteArray();
  auto popularityContest = QJsonDocument::fromJson(popularityContestStr).object();

  if(popularityContest.length()) {
    auto popularityContestStdMap = popularityContest.toVariantMap().toStdMap();

    auto best = std::max_element(popularityContestStdMap.begin(), popularityContestStdMap.end(),
                                 [](const std::pair<QString, QVariant> &a,
                                    const std::pair<QString, QVariant> &b) -> bool {
                                     return a.second < b.second;
                                 });
    task.model = best->first;
  }

  // check if model is actually available
  if(!kotkiModels.contains(task.model)) {
    qDebug() << QString("Language model %1 not available, switching to default 'ende'").arg(task.model);
    task.model = "ende";
    if(!kotkiModels.contains(task.model)) {
      // still not available? exit
      auto msg = QString("Language model %1 not available - did you install any? Exiting.").arg(task.model);
      QMessageBox msgBox;
      msgBox.setText(msg);
      msgBox.exec();
      throw std::runtime_error(msg.toStdString());
    }
  }

  // load transliteration database
  auto data = Utils::fileOpenQRC(":/assets/cyrillic-transliteration.json");
  auto doc = QJsonDocument::fromJson(data);
  auto obj = doc.object();
  this->transliteration_langs = obj.keys();

  this->queueTask(task);
  preloadModel = task.model;
}

void AppContext::queueTask(TranslationTask task) {
  m_tasks->clear();
  m_tasks->put(task);
}

void AppContext::onTranslationEnded(TranslationTask task) {
  translationTaskResult = task;
  if(task.hidden || !task.popularity) return;

  // increment popularity stats
  auto popularityContestStr = config()->get(ConfigKeys::PopularityContest).toByteArray();
  auto obj = QJsonDocument::fromJson(popularityContestStr).object();

  if(!obj.contains(task.model))
    obj[task.model] = 0;

  obj[task.model] = obj[task.model].toInt() + 1;

  QJsonDocument doc(obj);
  config()->set(ConfigKeys::PopularityContest, doc.toJson(QJsonDocument::Compact));
  config()->sync();
}

void AppContext::createConfigDirectory(const QString &dir) {
  QStringList createDirs({dir});
  for(const auto &d: createDirs) {
    if(!std::filesystem::exists(d.toStdString())) {
      qDebug() << QString("Creating directory: %1").arg(d);
      if (!QDir().mkpath(d))
        throw std::runtime_error("Could not create directory " + d.toStdString());
    }
  }
}

AppContext::~AppContext() {}
