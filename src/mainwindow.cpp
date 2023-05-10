#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "lib/debounce.h"

MainWindow::MainWindow(AppContext *ctx, QWidget *parent) :
    QMainWindow(parent),
    m_ctx(ctx),
    ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ui->toTxtLatin->hide();
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
  this->setupCompleter();

  QTimer::singleShot(100, [this](){
    connect(ui->fromTxt, &QTextEdit::textChanged, this, &MainWindow::onTextChanged);
    connect(ui->langBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onLangChanged(const QString&)));
    connect(m_ctx->translationThread, &TranslationThread::translationStarted, this, &MainWindow::onTranslationStarted);
    connect(m_ctx->translationThread, &TranslationThread::translationEnded, this, &MainWindow::onTranslationEnded);
    connect(ui->btnCopy, &QPushButton::pressed, this, &MainWindow::onCopyPressed);
    connect(ui->btnClear, &QPushButton::pressed, this, &MainWindow::onClearPressed);
    connect(ui->btnPaste, &QPushButton::pressed, this, &MainWindow::onPastePressed);

      if(!m_ctx->translationTaskResult.err) {
        // manually pass the initial translation task
        // when the result is completed before
        // the mainwindow even spawned
        this->onTranslationEnded(m_ctx->translationTaskResult);
      } else {
        ui->toTxt->setPlainText("Error");
      }
  });
}

void MainWindow::onCopyPressed() {
  QClipboard *clipboard = QGuiApplication::clipboard();
  auto toTxt = ui->toTxt->toPlainText();
  clipboard->setText(toTxt);
}

void MainWindow::onClearPressed() {
  ui->fromTxt->clear();
  ui->toTxt->clear();
  ui->toTxtLatin->clear();
}

void MainWindow::onPastePressed() {
  QClipboard *clipboard = QGuiApplication::clipboard();
  QString clipboardText = clipboard->text();
  ui->fromTxt->setText(clipboardText);
}

void MainWindow::setupCompleter() {
  auto exists = QFile::exists("/usr/share/dict/american-english");
  if(!exists) {
    qWarning() << "could not find /usr/share/dict/american-english so the completer is disabled, you need: 'apt install -y wamerican'";
    return;
  }

  QFile inputFile("/usr/share/dict/american-english");
  inputFile.open(QIODevice::ReadOnly);
  if (!inputFile.isOpen())
    return;

  QTextStream stream(&inputFile);
  for (QString line = stream.readLine();
       !line.isNull();
       line = stream.readLine()) {
    if(line.length() <= 2)
      continue;
    if(line.contains("'"))
      continue;

    m_dict << line.trimmed();
  }

  m_completerModel = new QStringListModel(m_dict, m_completer);
  m_completer = new QCompleter(this);
  m_completer->setModel(m_completerModel);
  m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  m_completer->setWrapAround(false);

  ui->fromTxt->setCompleter(m_completer);
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
  if(ui->fromTxt->toPlainText().length() > 0) {
    m_debouncedTranslation();
    return;
  }

  if(ui->toTxt->toPlainText().length() > 0)
    ui->toTxt->setPlainText("");

  if(ui->toTxtLatin->toPlainText().length() > 0)
    ui->toTxtLatin->setPlainText("");
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

  // transliteration
  auto lang = task.model.remove(0, 2);
  if(m_ctx->transliteration_langs.contains(lang)) {
    ui->toTxtLatin->show();
    ui->latinResultLine->show();
    auto transliteration = Utils::transliteration(lang, task.result);
    ui->toTxtLatin->setPlainText(transliteration);
  } else {
    ui->toTxtLatin->hide();
    ui->latinResultLine->hide();
  }
}

void MainWindow::onLangChanged(QString description) {
  for(const auto &model: m_translationModels) {
    std::string desc = model.at("description");
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

