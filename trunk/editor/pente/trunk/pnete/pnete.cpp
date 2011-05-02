#include <unistd.h>

#include <iostream>

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QPixmap>
#include <QSplashScreen>

#include <iostream>

#include "ui/MainWindow.hpp"

//! \todo inherit from QApplication and do these things in a class.
int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  Q_INIT_RESOURCE(resources);

  QCoreApplication::setApplicationName("pnete");
  //! \todo get svn revision.
  QCoreApplication::setApplicationVersion("0.1");
  QCoreApplication::setOrganizationDomain("itwm.fhg.de");
  QCoreApplication::setOrganizationName("Fraunhofer ITWM");

  QPixmap pixmap(":/pente.png");
  QSplashScreen splash(pixmap);
  splash.show();
  a.processEvents();

  QTranslator translator;
  translator.load("german", ":/localization/");
  a.installTranslator(&translator);

  QSettings settings;

  settings.beginGroup("transitionLibrary");

  if (argc > 1)
  {
    if (0 == strcmp ("--make-config" , argv[1]))
    {
      const char * PREFIX = getenv ("SDPA_HOME");

      if (0 == PREFIX)
      {
        std::cerr << "E: SDPA_HOME is not set, please export it first!" << std::endl;
        return EXIT_FAILURE;
      }

      char buffer[4096];

      snprintf (buffer, sizeof(buffer), "%s/lib", PREFIX);
      settings.setValue("basePath", buffer);
      settings.beginWriteArray("trustedPaths");
      settings.setArrayIndex(0);

      snprintf (buffer, sizeof(buffer), "%s/statoil", PREFIX);
      settings.setValue("path", buffer);
      settings.endArray();

      snprintf (buffer, sizeof(buffer), "%s/user", PREFIX);
      settings.beginWriteArray("userPaths");
      settings.setArrayIndex(0);
      settings.setValue("path", buffer);
      settings.endArray();
    }
  }

  if(!settings.contains("basePath"))
  {
    //! \todo error message, auto config creation, ...
    std::cerr << "There is no base path set for the transition library." << std::endl;
    std::cerr << "Please run " << argv[0] << " --make-config" << std::endl;
    return EXIT_FAILURE;
  }

  fhg::pnete::ui::MainWindow w;
  w.setTransitionLibraryPath(settings.value("basePath").toString());

  int numTrusted = settings.beginReadArray("trustedPaths");
  for(int i = 0; i < numTrusted; ++i)
  {
    settings.setArrayIndex(i);
    w.addTransitionLibraryUserPath(settings.value("path").toString(), true);
  }
  settings.endArray();

  int numUser = settings.beginReadArray("userPaths");
  for(int i = 0; i < numUser; ++i)
  {
    settings.setArrayIndex(i);
    w.addTransitionLibraryUserPath(settings.value("path").toString(), false);
  }
  settings.endArray();

  w.show();
  splash.finish(&w);
  return a.exec();
}
