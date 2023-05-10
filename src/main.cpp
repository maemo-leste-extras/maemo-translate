#include <QApplication>

#include <unistd.h>
#include <sys/types.h>
#include "mainwindow.h"
#include "ctx.h"

int main(int argc, char *argv[])
{
#ifdef DEBUG
  // For remote debugging (QEMU) with CLion, the environment variables need
  // to be correctly set such that e.g. dbus will work. We can execute
  // this hack to dump the environ to a file, then reads it below.
  // This snippet runs when `-D CMAKE_DEBUG_TYPE=Debug`

  // /bin/sh -c 'nohup /tmp/tmp.ewFCsXbjq2/cmake-build-debug/bin/maemo-translate >/dev/null 2>&1 &'; sleep 2; cat "/proc/`pidof maemo-translate`/environ" | tr "\0" "\n" > /home/user/env.sh; kill -9 "`pidof maemo-translate`"
  setuid(1000);
  QString path_env_file = "/home/user/env.sh";
  qDebug() << "trying to read ENV from" << path_env_file << ", if it exists";
  auto env_file = Utils::fileOpen(path_env_file);
  for(auto &line: Utils::barrayToString(env_file).split("\n")) {
    line = line.replace("export ", "");
    int pos = line.indexOf("=");
    auto key = line.left(pos);
    auto val = line.remove(0, pos + 1);

    if(val.startsWith("\""))
      val = val.mid(1);
    if(val.endsWith("\""))
      val = val.mid(0, val.length() - 1);

    if(val.isEmpty() || key.isEmpty()) continue;
    qputenv(key.toStdString().c_str(), val.toStdString().c_str());
  }
#endif

  qRegisterMetaType<TranslationTask>("TranslationTask");
  QApplication a(argc, argv);
  auto *ctx = new AppContext();
  ctx->isDebug = false;

  auto *mainWindow = new MainWindow(ctx);
  return QApplication::exec();
}
