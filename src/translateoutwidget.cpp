#include <QScreen>

#include "translateoutwidget.h"
#include "ui_translateoutwidget.h"

#include "ctx.h"

TranslateOutWidget::TranslateOutWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TranslateOutWidget) {
  ui->setupUi(this);
  ui->toTxtLatin->hide();
  ui->toTxt->setStyleSheet("QPlainTextEdit { background-color: #443e41; border: 6px solid #443e41; border-radius: 6px; }");
  ui->toTxtLatin->setStyleSheet("QPlainTextEdit { background-color: #443e41; border: 6px solid #443e41; border-radius: 6px; }");

  ui->toTxt->document()->setDocumentMargin(12);
  ui->toTxtLatin->document()->setDocumentMargin(12);

  QTimer::singleShot(100, [this]() {
    auto *ctx = AppContext::instance();
    connect(ctx->translationThread, &TranslationThread::translationStarted, this, &TranslateOutWidget::onTranslationStarted);
    connect(ctx->translationThread, &TranslationThread::translationEnded, this, &TranslateOutWidget::onTranslationEnded);

    if(!ctx->translationTaskResult.err) {
      // manually pass the initial translation task
      // when the result is completed before
      // the mainwindow even spawned
      this->onTranslationEnded(ctx->translationTaskResult);
    } else {
      ui->toTxt->setPlainText("Error");
    }
  });
}

void TranslateOutWidget::onTranslationStarted() const {
  QPalette pa(palette());
  pa.setColor(QPalette::Base, Qt::black);
  pa.setColor(QPalette::Text, Qt::red);
  // ui->toTxt->setPalette(pa);
}

void TranslateOutWidget::onTranslationEnded(TranslationTask task) const {
  if(task.hidden) return;
  QPalette pa(palette());
  pa.setColor(QPalette::Base, Qt::black);
  pa.setColor(QPalette::Text, Qt::white);
  ui->toTxt->setPalette(pa);
  ui->toTxt->setPlainText(task.result);

  // transliteration
  auto lang = task.model.remove(0, 2);
  auto *ctx = AppContext::instance();
  if(ctx->transliteration_langs.contains(lang)) {
    ui->toTxtLatin->show();
    auto transliteration = Utils::transliteration(lang, task.result);
    ui->toTxtLatin->setPlainText(transliteration);
  } else {
    ui->toTxtLatin->hide();
  }
}

void TranslateOutWidget::clear() {
  ui->toTxt->setPlainText("");
  ui->toTxtLatin->setPlainText("");
}

QString TranslateOutWidget::getText() {
  return ui->toTxt->toPlainText();
}

TranslateOutWidget::~TranslateOutWidget() {
  delete ui;
}
