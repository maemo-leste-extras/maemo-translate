#include <QObject>
#include <QDir>
#include <QStandardPaths>

#include "ctx.h"

using namespace std::chrono;

void TranslationThread::run() {
  while(true) {
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

  // setup translation thread
  m_tasks = QSharedPointer<MessageQueue>::create();
  translationThread = new TranslationThread(m_tasks);
  connect(translationThread, &TranslationThread::translationStarted, this, &AppContext::translationStarted);
  connect(translationThread, &TranslationThread::translationEnded, this, &AppContext::translationEnded);
  translationThread->start();
}

void AppContext::queueTask(TranslationTask task) {
  m_tasks->clear();
  m_tasks->put(task);
}

void AppContext::createConfigDirectory(const QString &dir) {
  QStringList createDirs({dir});
  for(const auto &d: createDirs) {
    if(!Utils::dirExists(d)) {
      qDebug() << QString("Creating directory: %1").arg(d);
      if (!QDir().mkpath(d))
        throw std::runtime_error("Could not create directory " + d.toStdString());
    }
  }
}

AppContext::~AppContext() {}
