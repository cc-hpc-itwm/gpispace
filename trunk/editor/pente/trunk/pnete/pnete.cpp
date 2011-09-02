#include <unistd.h>

#include <iostream>

#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QPixmap>
#include <QSplashScreen>
#include <QLocale>
#include <QLibraryInfo>

#include <iostream>

#include "ui/MainWindow.hpp"

#define BE_PENTE 1

//! \todo inherit from QApplication and do these things in a class.
int main(int argc, char *argv[])
{
                                                                                // hardcoded constants
  QApplication a(argc, argv);
  Q_INIT_RESOURCE(resources);

  QCoreApplication::setApplicationName("pnete");
  //! \todo get svn revision.
  QCoreApplication::setApplicationVersion("0.1");
  QCoreApplication::setOrganizationDomain("itwm.fhg.de");
  QCoreApplication::setOrganizationName("Fraunhofer ITWM");

#ifdef BE_PENTE
  QPixmap pixmap(":/pente.png");
  QSplashScreen splash(pixmap);
  splash.show();
  a.processEvents();
#endif

  QTranslator qtTranslator;
  QString locale (QLocale::system().name());

  qtTranslator.load ("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  a.installTranslator(&qtTranslator);

  QTranslator translator;
  translator.load(locale, ":/localization/");
  a.installTranslator(&translator);

  QSettings settings;

  settings.beginGroup("transitionLibrary");

  QStringList arguments = QCoreApplication::arguments();
  if(arguments.contains("--make-config"))
  {
    if(arguments.size() <= arguments.indexOf("--make-config") + 1)
    {
      std::cerr << "Please also specify a path containing lib/, statoil/ and user/!" << std::endl;
      return EXIT_FAILURE;
    }

    QString pnetedir = arguments.at(arguments.indexOf("--make-config") + 1);

    settings.setValue("basePath", QString("%1/lib").arg(pnetedir));

    //! \todo this should not be default!
    settings.beginWriteArray("trustedPaths");
    settings.setArrayIndex(0);
    settings.setValue("path", QString("%1/statoil").arg(pnetedir));
    settings.endArray();

    settings.beginWriteArray("userPaths");
    settings.setArrayIndex(0);
    settings.setValue("path", QString("%1/user").arg(pnetedir));
    settings.endArray();
  }

  QString load;

  if (arguments.contains("--load"))
    {
      if(arguments.size() <= arguments.indexOf("--load") + 1)
        {
          std::cerr << "--load requires a path" << std::endl;
          return EXIT_FAILURE;
        }

      load = arguments.at (arguments.indexOf("--load") + 1);
    }

  if(!settings.contains("basePath"))
  {
    //! \todo error message.
    std::cerr << "There is no base path set for the transition library." << std::endl;
    std::cerr << "Please run \"" << qPrintable(arguments.at(0)) << " --make-config <path>\", where path contains lib." << std::endl;
    return EXIT_FAILURE;
  }

  fhg::pnete::ui::MainWindow w (load);
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
#ifdef BE_PENTE
  splash.finish(&w);
#endif

  return a.exec();
}

#undef BE_PENTE
