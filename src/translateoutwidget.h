#pragma once
#include <QCompleter>
#include <QCheckBox>
#include <QPushButton>
#include <QStringList>
#include <QClipboard>
#include <QScroller>
#include <QStringListModel>
#include <QTimer>
#include <QEasingCurve>
#include <QMessageBox>
#include <QWidget>
#include <QMenu>

#include "lib/utils.h"

namespace Ui {
  class TranslateOutWidget;
}

struct TranslationTask;

class TranslateOutWidget final : public QWidget {
Q_OBJECT

public:
  explicit TranslateOutWidget(QWidget *parent = nullptr);
  ~TranslateOutWidget() override;

  void clear();
  QString getText();
private slots:
  void onTranslationStarted() const;
  void onTranslationEnded(TranslationTask task) const;
private:
  Ui::TranslateOutWidget *ui;
};
