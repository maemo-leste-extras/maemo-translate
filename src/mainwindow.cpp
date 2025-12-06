#include <QtMaemo5/QMaemo5ListPickSelector>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "lib/debounce.h"

MainWindow::MainWindow(AppContext *ctx, QWidget *parent) :
    QMainWindow(parent),
    m_ctx(ctx),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ui->menuBar->hide();

  setProperty("X-Maemo-StackedWindow", 1);
  setProperty("X-Maemo-Orientation", 2);

  const auto geo = QGuiApplication::primaryScreen()->geometry();

  connect(m_ctx->translateInWidget, &TranslateInWidget::textChanged, [this](const QString &text) {
    if (text.length() == 0)
      m_ctx->translateOutWidget->clear();
  });

  ui->contentFrame->setVisible(true);

  this->setupLanguageModelPicker();
  this->show();

  connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onOpenAboutWindow);
  connect(ui->actionExit, &QAction::triggered, this, &MainWindow::onQuitApplication);

  connect(ui->btnCopy, &QPushButton::pressed, this, &MainWindow::onCopyPressed);
  connect(ui->btnClear, &QPushButton::pressed, this, &MainWindow::onClearPressed);
  connect(ui->btnPaste, &QPushButton::pressed, this, &MainWindow::onPastePressed);

  if (geo.width() > geo.height()) setupContent(Orientation::Vertical);
  else setupContent(Orientation::Vertical);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  const QSize geo = event->size();
  if (geo.width() > geo.height()) setupContent(Orientation::Horizontal);
  else setupContent(Orientation::Vertical);
  QMainWindow::resizeEvent(event);
}

void MainWindow::onCopyPressed() const {
  QClipboard *clipboard = QGuiApplication::clipboard();
  const auto text = m_ctx->translateOutWidget->getText();
  clipboard->setText(text);
}

void MainWindow::onClearPressed() const {
  m_ctx->translateInWidget->clear();
  m_ctx->translateOutWidget->clear();
}

void MainWindow::onPastePressed() const {
  const QClipboard *clipboard = QGuiApplication::clipboard();
  const QString clipboardText = clipboard->text();
  if (clipboardText.length() == 0) return;
  m_ctx->translateInWidget->setText(clipboardText);
}

void MainWindow::setupLanguageModelPicker() {
  auto models = m_ctx->kotki->listModels();
  QStringList modelKeys;

  QMaemo5ListPickSelector *selector = new QMaemo5ListPickSelector;
  QStandardItemModel *model = new QStandardItemModel(0, 1, selector);
  QString value;

  for(const auto &[key, value]: models) {
    auto desc = value.at("description");
    model->appendRow(new QStandardItem(QString::fromStdString(desc)));
  }
  selector->setModel(model);

  // set combobox to default language model
  for(int i = 0; i != m_ctx->translationModels.size(); i++) {
    auto name = m_ctx->translationModels[i].at("name");
    if(name == m_ctx->currentModel.toStdString()) {
      selector->setCurrentIndex(i);
      break;
    }
  }

  ui->langBox->setPickSelector(selector);
  langBoxSelector = selector;
  connect(langBoxSelector, SIGNAL(selected(const QString&)), this, SLOT(onLangChanged(const QString&)));
}

void MainWindow::onLangChanged(const QString &description) const {
  for(const auto &model: m_ctx->translationModels) {
    std::string desc = model.at("description");
    if(desc == description.toStdString()) {
      m_ctx->currentModel = QString::fromStdString(model.at("name"));
    }
  }

  m_ctx->onQueueTask(m_ctx->translateInWidget->getText());
}

void MainWindow::setupContent(const Orientation orientation) {
  auto *w = ui->contentFrame;
  if (w->layout())
    delete w->layout();

  w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  if (orientation == Orientation::Horizontal) {
    QHBoxLayout *layout = new QHBoxLayout(w);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(12);

    layout->addWidget(m_ctx->translateInWidget);
    layout->addWidget(m_ctx->translateOutWidget);

    w->setLayout(layout);
  } else {
    QVBoxLayout *layout = new QVBoxLayout(w);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(12);

    layout->addWidget(m_ctx->translateInWidget);
    layout->addWidget(m_ctx->translateOutWidget);
    w->setLayout(layout);
  }
}

void MainWindow::onOpenAboutWindow() {
  m_about = new About(m_ctx, this);
  m_about->show();
}

void MainWindow::onQuitApplication() {
  this->close();
}

MainWindow::~MainWindow() {
  delete ui;
}

