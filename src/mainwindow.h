#pragma once

#include <iostream>

#include <QMainWindow>
#include <QCompleter>
#include <QPushButton>
#include <QClipboard>
#include <QStringListModel>
#include <QTimer>
#include <QDebug>
#include <QQueue>
#include <QThread>
#include <QMutex>

#include "ctx.h"
#include "about.h"
#include "lib/utils.h"

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
  void setupCompleter();
  ~MainWindow();

signals:
  void startTranslation(TranslationTask task);

public slots:
    void onOpenAboutWindow();
    void onQuitApplication();

private slots:
  void onTextChanged();
  void onTranslationStarted();
  void onTranslationEnded(TranslationTask task);
  void onLangChanged(QString description);
  void onPastePressed();
  void onClearPressed();
  void onCopyPressed();

private:
  Ui::MainWindow *ui;
  AppContext *m_ctx = nullptr;
  std::function<void()> m_debouncedTranslation;
  std::vector<std::map<std::string, std::string>> m_translationModels;
  QString m_currentModel = "ende";

  About *m_about = nullptr;
  QStringList m_dict;
  QStringListModel *m_completerModel = nullptr;
  QCompleter *m_completer = nullptr;
};
