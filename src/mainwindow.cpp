#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "lib/debounce.h"

MainWindow::MainWindow(AppContext *ctx, QWidget *parent) :
    QMainWindow(parent),
    m_ctx(ctx),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  m_currentModel = m_ctx->preloadModel;

  QPixmap p(":assets/logo.jpg");
  qDebug() << "width: " << this->size().width();
  qDebug() << "height: " << this->size().height();

  // splash image, droid 4 reports 1346x922 which is
  // not correct (?). we manually set it, and thus assume
  // droid4, which is not necessarily the case ... oh well
  auto width = this->size().width();
  auto height = this->size().height();
  width = 960;
  height = 540;
  ui->img->setPixmap(p.scaled(QSize(width, height)));
  ui->mainFrame->setVisible(false);
  ui->imgFrame->setVisible(true);

  // splash duration
  QTimer::singleShot(2000, [this](){
    ui->mainFrame->setVisible(true);
    ui->imgFrame->setVisible(false);
  });

  m_debouncedTranslation = QFunctionUtils::Debounce([=]{
    this->queueTask();
  }, 500);

  this->setupUIModels();
  this->show();

  QTimer::singleShot(100, [this](){
    connect(ui->fromTxt, &QPlainTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(ui->langBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onLangChanged(const QString&)));
    connect(m_ctx->translationThread, &TranslationThread::translationStarted, this, &MainWindow::onTranslationStarted);
    connect(m_ctx->translationThread, &TranslationThread::translationEnded, this, &MainWindow::onTranslationEnded);
  });
}

void MainWindow::setupUIModels() {
  ui->langBox->clear();
  auto models = m_ctx->kotki->listModels();
  QStringList modelKeys;

  // fill combobox
  for(const auto &[key, value]: models) {
    auto desc = value.at("description");
    modelKeys.append(QString::fromStdString(desc));
    m_translationModels.emplace_back(value);
  }
  ui->langBox->addItems(modelKeys);

  // set combobox to whatever is the default language model
  for(int i = 0; i != m_translationModels.size(); i++) {
    auto name = m_translationModels[i].at("name");
    if(name == m_currentModel.toStdString()) {
      ui->langBox->setCurrentIndex(i);
      break;
    }
  }
}

void MainWindow::onTextChanged() {
  m_debouncedTranslation();
}

void MainWindow::onTranslationStarted() {
  QPalette pa(palette());
  pa.setColor(QPalette::Base, Qt::black);
  pa.setColor(QPalette::Text, Qt::red);
  ui->toTxt->setPalette(pa);
}

void MainWindow::onTranslationEnded(TranslationTask task) {
  if(task.hidden) return;
  QPalette pa(palette());
  pa.setColor(QPalette::Base, Qt::black);
  pa.setColor(QPalette::Text, Qt::white);
  ui->toTxt->setPalette(pa);
  ui->toTxt->setPlainText(task.result);
}

void MainWindow::onLangChanged(QString description) {
  for(const auto &model: m_translationModels) {
    string desc = model.at("description");
    if(desc == description.toStdString()) {
      m_currentModel = QString::fromStdString(model.at("name"));
    }
  }

  this->queueTask();
}

void MainWindow::queueTask() {
  TranslationTask task;
  task.source = ui->fromTxt->toPlainText();
  task.kotki = m_ctx->kotki;
  task.model = m_currentModel;
  m_ctx->queueTask(task);
}


MainWindow::~MainWindow() {
  delete ui;
}

