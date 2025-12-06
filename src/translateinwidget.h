#pragma once
#include <QCompleter>
#include <QCheckBox>
#include <QPushButton>
#include <QStringList>
#include <QClipboard>
#include <QIODevice>
#include <QFile>
#include <QScroller>
#include <QStringListModel>
#include <QTimer>
#include <QEasingCurve>
#include <QMessageBox>
#include <QWidget>
#include <QMenu>

#include "lib/utils.h"

namespace Ui {
  class TranslateInWidget;
}

class TranslateInWidget final : public QWidget {
Q_OBJECT

public:
  explicit TranslateInWidget(QWidget *parent = nullptr);
  ~TranslateInWidget() override;
  QString getText();
  void setText(const QString &text);
  void clear();
signals:
  void textChanged(QString text);
private slots:
  void onTextChanged() const;
private:
  Ui::TranslateInWidget *ui;
  std::function<void()> m_debouncedTranslation;
  void setupCompleter();

  QStringList m_dict;
  QStringListModel *m_completerModel = nullptr;
  QCompleter *m_completer = nullptr;
};
