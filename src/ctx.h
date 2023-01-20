#pragma once

#include <QObject>
#include <QMainWindow>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QTimer>
#include <QDebug>
#include <QQueue>
#include <QThread>
#include <QMutex>

#include "lib/debounce.h"
#include "lib/blocking_queue.h"

#include "lib/config.h"
#include "kotki/kotki.h"

struct TranslationTask {
  QString source;
  QString result;
  QString model = "ende";
  unsigned int timing = 0; // in milliseconds
  Kotki *kotki = nullptr;
  bool hidden = false;  // update UI?
  bool popularity = true;  // update popularity contest?
};
Q_DECLARE_METATYPE(TranslationTask);

typedef BlockingQueue<TranslationTask> MessageQueue;

class TranslationThread: public QThread
{
Q_OBJECT

public:
  TranslationThread(QSharedPointer<MessageQueue> tasks) : tasks(tasks) {}
  virtual void run() override;

signals:
  void translationStarted();
  void translationEnded(TranslationTask task);
public:
  QSharedPointer<MessageQueue> tasks;
};


class AppContext : public QObject {
Q_OBJECT

public:
  explicit AppContext();
  ~AppContext() override;

  bool isDebug;
  QString preloadModel;
  QString configDirectory;
  QString configRoot;
  QString homeDir;
  Kotki *kotki;
  TranslationThread *translationThread;

  void queueTask(TranslationTask task);

signals:
  void translationStarted();
  void translationEnded(TranslationTask task);
  void modelsAvailable(QStringList models);

private slots:
    void onTranslationEnded(TranslationTask task);

private:
  QSharedPointer<MessageQueue> m_tasks;
  static void createConfigDirectory(const QString &dir);
};
