#include <QScreen>
#include <QTextEdit>

#include "translateinwidget.h"
#include "ui_translateinwidget.h"

#include "lib/debounce.h"

TranslateInWidget::TranslateInWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TranslateInWidget) {
    ui->setupUi(this);

  ui->fromTxt->setText("Hello World");
  connect(ui->fromTxt, &QTextEdit::textChanged, this, &TranslateInWidget::onTextChanged);

  ui->fromTxt->setStyleSheet("TextEdit { background-color: #443e41; border: 6px solid #443e41; border-radius: 6px; }");
  ui->fromTxt->document()->setDocumentMargin(12);

  m_debouncedTranslation = QFunctionUtils::Debounce([=]{
    emit textChanged(ui->fromTxt->toPlainText());
  }, 500);

  this->setupCompleter();
}

void TranslateInWidget::setupCompleter() {
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
  for (QString line = stream.readLine(); !line.isNull(); line = stream.readLine()) {
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

void TranslateInWidget::clear() {
  ui->fromTxt->setPlainText("");
}

void TranslateInWidget::setText(const QString &text) {
  ui->fromTxt->setPlainText(text);
}

QString TranslateInWidget::getText() {
  return ui->fromTxt->toPlainText();
}

void TranslateInWidget::onTextChanged() const {
  if (m_debouncedTranslation != nullptr)
    m_debouncedTranslation();
}

TranslateInWidget::~TranslateInWidget() {
  delete ui;
}
