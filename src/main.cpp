#include <QApplication>

#include <unistd.h>
#include <sys/types.h>
#include "mainwindow.h"
#include "ctx.h"

#ifdef DEBUG
  #include "lib/clion_debug.h"
#endif

int main(int argc, char *argv[])
{
#ifdef DEBUG
  clion_debug_setup();
#endif
  qputenv("QT_STYLE_OVERRIDE", QByteArray("maemo5"));
  qputenv("QT_QPA_PLATFORMTHEME", QByteArray("maemo5"));
  qputenv("QT_QPA_PLATFORM", QByteArray("maemo"));

  qRegisterMetaType<TranslationTask>("TranslationTask");
  QApplication a(argc, argv);
  auto *ctx = new AppContext();
  ctx->isDebug = false;

  auto *mainWindow = new MainWindow(ctx);
  return QApplication::exec();
}
