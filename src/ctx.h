#pragma once

#include <QObject>
#include <QMainWindow>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QQueue>
#include <QThread>
#include <QMutex>

#include "translateinwidget.h"
#include "translateoutwidget.h"
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
  bool err = false;
};
Q_DECLARE_METATYPE(TranslationTask);

typedef BlockingQueue<TranslationTask> MessageQueue;



class TranslationThread final : public QThread {
Q_OBJECT

public:
  explicit TranslationThread(const QSharedPointer<MessageQueue> &tasks) : tasks(tasks) {}
  virtual void run() override;

signals:
  void translationStarted();
  void translationEnded(TranslationTask task);
public:
  QSharedPointer<MessageQueue> tasks;
};

class AppContext;
extern AppContext* CTX;

class AppContext final : public QObject {
Q_OBJECT

public:
  explicit AppContext();
  ~AppContext() override;
  static AppContext* instance() {
    if (CTX == nullptr) throw std::runtime_error("ctx is null");
    return CTX;
  }

  void onQueueTask(const QString &text);

  QString currentModel = "ende";
  std::vector<std::map<std::string, std::string>> translationModels;

  bool isDebug;
  QString preloadModel;
  QString configDirectory;
  QString configRoot;
  QString homeDir;
  Kotki *kotki;
  QStringList kotkiModels;
  TranslationThread *translationThread;
  QStringList transliteration_langs;
  TranslationTask translationTaskResult;
  TranslateInWidget *translateInWidget = nullptr;
  TranslateOutWidget *translateOutWidget = nullptr;

signals:
  void translationStarted();
  void translationEnded(TranslationTask task);
  void modelsAvailable(QStringList models);

private slots:
    void onTranslationEnded(TranslationTask task);

private:
  QSharedPointer<MessageQueue> m_tasks;

  void _queueTask(TranslationTask task);
  static void createConfigDirectory(const QString &dir);
};
