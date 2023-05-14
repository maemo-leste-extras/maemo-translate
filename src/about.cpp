#include <QPixmap>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QGroupBox>
#include <QFileDialog>
#include <QCheckBox>

#include "about.h"

#include "ui_about.h"

About * About::pAbout = nullptr;

About::About(AppContext *ctx, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::About),
    m_ctx(ctx) {
  pAbout = this;
  ui->setupUi(this);
}

AppContext *About::getContext(){
  return pAbout->m_ctx;
}

void About::closeEvent(QCloseEvent *event) {
  QWidget::closeEvent(event);
}

About::~About() {
  delete ui;
}
