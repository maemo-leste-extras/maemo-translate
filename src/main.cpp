#include <QApplication>

#include "mainwindow.h"
#include "ctx.h"

int main(int argc, char *argv[])
{
  qRegisterMetaType<TranslationTask>("TranslationTask");
  QApplication a(argc, argv);
  auto *ctx = new AppContext();
  ctx->isDebug = false;

  auto *mainWindow = new MainWindow(ctx);
  return QApplication::exec();
}
