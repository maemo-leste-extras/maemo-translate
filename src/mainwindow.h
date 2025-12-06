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

#include <QtMaemo5/QMaemo5ListPickSelector>

namespace Ui {
  class MainWindow;
}

class AppContext;
class MainWindow final : public QMainWindow {
Q_OBJECT

enum class Orientation {
  Horizontal,
  Vertical
};

public:
  explicit MainWindow(AppContext *ctx, QWidget *parent = nullptr);
  void setupLanguageModelPicker();
  ~MainWindow();

  void setupContent(Orientation orientation);

signals:
  void startTranslation(TranslationTask task);

public slots:
  void onOpenAboutWindow();
  void onQuitApplication();

private slots:
  void onLangChanged(const QString& description) const;
  void onPastePressed() const;
  void onClearPressed() const;
  void onCopyPressed() const;

protected:
  void resizeEvent(QResizeEvent *event) override;

private:
  Ui::MainWindow *ui;
  AppContext *m_ctx = nullptr;

  About *m_about = nullptr;
  QMaemo5ListPickSelector *langBoxSelector = nullptr;
};
