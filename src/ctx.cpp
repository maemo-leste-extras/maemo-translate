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
    auto then = duration_cast< milliseconds >(system_clock::now().time_since_epoch());

    task.result = QString::fromStdString(task.kotki->translate(
      message.toStdString(),
      task.model.toStdString()));

    auto now = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
    auto took = duration_cast<milliseconds>(now - then);
    task.timing = took.count();

    task.err = task.result.isEmpty();
    emit translationEnded(task);
  }
}

AppContext* CTX = nullptr;

AppContext::AppContext() {
  CTX = this;
  configRoot = QDir::homePath();
  homeDir = QDir::homePath();
  configDirectory = QString("%1/.config/%2/").arg(configRoot, QCoreApplication::applicationName());
  createConfigDirectory(configDirectory);

  kotki = new Kotki();
  kotki->scan();

  for (const auto &[fst, snd]: kotki->listModels())
    kotkiModels << QString::fromStdString(fst);

  // setup translation thread
  m_tasks = QSharedPointer<MessageQueue>::create();
  translationThread = new TranslationThread(m_tasks);
  connect(translationThread, &TranslationThread::translationStarted, this, &AppContext::translationStarted);
  connect(translationThread, &TranslationThread::translationEnded, this, &AppContext::translationEnded);
  connect(translationThread, &TranslationThread::translationEnded, this, &AppContext::onTranslationEnded);
  translationThread->start();

  // preload
  TranslationTask task;
  task.source = "Hello World";
  task.kotki = kotki;
  task.hidden = false;
  task.popularity = false;
  task.model = "ende";

  // preload most popular language model
  auto popularityContestStr = config()->get(ConfigKeys::PopularityContest).toByteArray();
  auto popularityContest = QJsonDocument::fromJson(popularityContestStr).object();

  if(popularityContest.length()) {
    auto popularityContestStdMap = popularityContest.toVariantMap().toStdMap();

    auto best = std::max_element(popularityContestStdMap.begin(),
                                 popularityContestStdMap.end(),
      [](const auto &a, const auto &b) {
        return QVariant::compare(a.second, b.second) < 0;
      });
    task.model = best->first;
  }

  // check if the preferred startup model is actually available
  if(!kotkiModels.contains(task.model)) {
    qDebug() << QString("Language model %1 not available, switching to first available").arg(task.model);
    task.model = kotkiModels.at(0);
  }

  // load transliteration database
  const auto data = Utils::fileOpenQRC(":/assets/cyrillic-transliteration.json");
  const auto doc = QJsonDocument::fromJson(data);
  const auto obj = doc.object();
  this->transliteration_langs = obj.keys();

  this->_queueTask(task);
  preloadModel = task.model;
  currentModel = preloadModel;

  auto models = kotki->listModels();
  for(const auto &[key, value]: models) {
    auto desc = value.at("description");
    translationModels.emplace_back(value);
  }

  translateInWidget = new TranslateInWidget;
  translateOutWidget = new TranslateOutWidget;
  connect(translateInWidget, &TranslateInWidget::textChanged, this, &AppContext::onQueueTask);
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

void AppContext::onQueueTask(const QString &text) {
  TranslationTask task;
  task.source = text;
  task.kotki = kotki;
  task.model = currentModel;

  _queueTask(task);
}

void AppContext::_queueTask(TranslationTask task) {
  m_tasks->clear();
  m_tasks->put(task);
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
