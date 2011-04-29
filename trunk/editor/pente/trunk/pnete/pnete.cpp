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
  //! \todo embed?
  translator.load("german", ":/localization/");

  QSettings settings;

  settings.beginGroup("transitionLibrary");

#ifdef CREATE_A_NEW_SETTINGS_FILE_BY_CHANGING_THE_PATHS_IN_HERE
  settings.setValue("basePath", "/Users/berndlorwald/Documents/Arbeit/SDPA/svn/trunk/editor/pente/trunk/demo/lib");
  settings.beginWriteArray("trustedPaths");
  settings.setArrayIndex(0);
  settings.setValue("path", "/Users/berndlorwald/Documents/Arbeit/SDPA/svn/trunk/editor/pente/trunk/demo/statoil");
  settings.endArray();

  settings.beginWriteArray("userPaths");
  settings.setArrayIndex(0);
  settings.setValue("path", "/Users/berndlorwald/Documents/Arbeit/SDPA/svn/trunk/editor/pente/trunk/demo/user");
  settings.endArray();
#endif

  if(!settings.contains("basePath"))
  {
    //! \todo error message, auto config creation, ...
    std::cerr << "There is no base path set for the transition library. Fix the config file, thanks." << std::endl;
    return -1;
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
