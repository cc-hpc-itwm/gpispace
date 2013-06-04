#include <gspc/mon/server.hpp>

#include <sysexits.h>

#include <QApplication>
#include <QDebug>
#include <QString>
#include <QFile>
#include <QDir>

int main (int argc, char** argv)
{
  QApplication app (argc, argv);

  if (argc != 5)
  {
    qDebug() << argv[0] << "workdir port hostlist hookdir";
    return EX_USAGE;
  }

  const QString workdir (argv[1]);
  const int port (QString (argv[2]).toInt());
  const QString hostlist (argv[3]);
  const QString hookdir (argv[4]);

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

  const gspc::mon::server s ( workdir
                            , port
                            , hostlist
                            , hookdir
                            );

  return app.exec();
}
