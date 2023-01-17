#pragma once

#include <iostream>

#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include <QQueue>
#include <QThread>
#include <QMutex>

#include "ctx.h"

namespace Ui {
class MainWindow;
}

class AppContext;
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(AppContext *ctx, QWidget *parent = nullptr);
  void setupUIModels();
  void queueTask();
  ~MainWindow();

signals:
  void startTranslation(TranslationTask task);

private slots:
  void onTextChanged();
  void onTranslationStarted();
  void onTranslationEnded(TranslationTask task);
  void onLangChanged(QString description);

private:
  Ui::MainWindow *ui;
  AppContext *m_ctx = nullptr;
  std::function<void()> m_debouncedTranslation;
  std::vector<std::map<std::string, std::string>> m_translationModels;
  QString m_currentModel = "ende";
};
