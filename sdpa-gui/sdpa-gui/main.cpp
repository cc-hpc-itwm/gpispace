#include <iostream>

#include "monitorwindow.hpp"

#include <fhg/revision.hpp>

#include <QApplication>
#include <QtCore/QSettings>
#include <QtCore/QString>

namespace
{
  void maybe_set
    (QSettings& settings, const QString& key, const QVariant& value)
  {
    if (!settings.value (key).isValid())
    {
      settings.setValue (key, value);
    }
  }

  void maybe_set_default_settings()
  {
    QSettings settings;

    settings.beginGroup ("gantt");

    maybe_set (settings, "created", QColor (128, 128, 128));
    maybe_set (settings, "started", QColor (255, 255, 0));
    maybe_set (settings, "finished", QColor (0, 200, 0));
    maybe_set (settings, "failed", QColor (255, 0, 0));
    maybe_set (settings, "cancelled", QColor (165, 42, 42));

    settings.endGroup();
  }
}

int main (int ac, char *av[])
{
  unsigned short exe_port (0);
  unsigned short log_port (2438);

  if (ac < 2)
  {
    std::cerr << "usage: " << av[0] << " gui-port [log-port]" << std::endl;
    return 1;
  }

  exe_port = QString (av[1]).toUShort();
  if (ac > 2)
    log_port = QString (av[2]).toUShort();

  QApplication a (ac, av);

  QApplication::setApplicationName ("sdpa-gui");
  QApplication::setApplicationVersion ( QString (fhg::project_version())
                                      .append (" - ")
                                      .append (fhg::project_revision())
                                      );
  QApplication::setOrganizationDomain ("itwm.fraunhofer.de");
  QApplication::setOrganizationName ("Fraunhofer ITWM");

  maybe_set_default_settings();

  MonitorWindow w (exe_port, log_port);
  w.show();

  return a.exec();
}
