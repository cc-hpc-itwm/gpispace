#include <gspc/mon/server.hpp>

#include <QApplication>
#include <QDebug>
#include <QString>
#include <QFile>

int main (int argc, char** argv)
{
  QApplication app (argc, argv);

  if (argc != 3)
  {
    qDebug() << argv[0] << "port hostlist";
    return -1;
  }

  const int port (QString (argv[1]).toInt());
  const QString hostlist (argv[2]);

  if (!QFile::exists (hostlist))
  {
    qDebug() << "hostlist" << hostlist << "does not exist";
    return -2;
  }

  const gspc::mon::server s (port, hostlist);

  app.exec();
}
