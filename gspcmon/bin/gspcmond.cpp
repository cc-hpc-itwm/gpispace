#include <gspc/mon/server.hpp>

#include <sysexits.h>

#include <QApplication>
#include <QDebug>
#include <QString>
#include <QFile>
#include <QDir>
#include <QThreadPool>

int main (int argc, char** argv)
{
  QApplication app (argc, argv);

  QThreadPool::globalInstance ()->setMaxThreadCount (8);

  if (argc < 4)
  {
    qDebug() << argv[0] << "port hostlist hookdir [workdir]";
    return EX_USAGE;
  }

  const int port (QString (argv[1]).toInt());
  const QString hostlist (argv[2]);
  const QString hookdir (argv[3]);
  QString workdir;
  if (argc > 4)
    workdir = argv [4];

  if (!QFile::exists (hostlist))
  {
    qDebug() << "hostlist" << hostlist << "does not exist";
    return EX_NOINPUT;
  }

  if (!QDir (hookdir).exists ())
  {
    qDebug() << "hookdir" << hookdir << "does not exist";
    return EX_NOINPUT;
  }

  const gspc::mon::server s ( port
                            , hostlist
                            , hookdir
                            , workdir
                            );

  return app.exec();
}
