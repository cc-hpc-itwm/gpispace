// bernd.loerwald@itwm.fraunhofer.de

#include "pnete.hpp"

#include <iostream>
#include <stdexcept>

#include <QSettings>
#include <QPixmap>
#include <QLocale>
#include <QLibraryInfo>

#include "ui/MainWindow.hpp"

#define BE_PENTE 1

int main (int argc, char *argv[])
{
  Q_INIT_RESOURCE (resources);

  fhg::pnete::PetriNetEditor pente (argc, argv);
  pente.startup ();
  return pente.exec ();
}

namespace fhg
{
  namespace pnete
  {
    void PetriNetEditor::showSplashScreen ()
    {
      _splash.show ();
      processEvents ();
    }

    void PetriNetEditor::hideSplashScreen ()
    {
      _splash.finish (_mainWindow);
    }

    PetriNetEditor::PetriNetEditor (int & argc, char *argv[])
    : QApplication (argc, argv)
    , _splash (QPixmap (":/pente.png"))
    , _mainWindow (NULL)
    {
      setApplicationName ("pnete");
      //! \todo Get SVN revision.
      setApplicationVersion ("0.1");
      setOrganizationDomain ("itwm.fhg.de");
      setOrganizationName ("Fraunhofer ITWM");
    }

    void PetriNetEditor::startup ()
    {
#ifdef BE_PENTE
      showSplashScreen ();
#endif

      setupLocalization ();
      processCommandLine ();
      createMainWindow ();
      createTransitionLibrary ();

#ifdef BE_PENTE
      hideSplashScreen();
#endif
    }

    PetriNetEditor::~PetriNetEditor ()
    {
      _mainWindow->hide();
      //! \todo This segfaults.
      delete _mainWindow;
      _mainWindow = NULL;
    }

    void PetriNetEditor::setupLocalization ()
    {
      const QString& locale (QLocale::system().name());

      _qtTranslator.load ( "qt_" + locale
                        , QLibraryInfo::location(QLibraryInfo::TranslationsPath));
      installTranslator (&_qtTranslator);

      _penteTranslator.load (locale, ":/localization/");
      installTranslator (&_penteTranslator);
    }

    void PetriNetEditor::processCommandLine ()
    {
      const QStringList& args (arguments ());
      if (args.contains ("--make-config"))
      {
        QSettings settings;
        settings.beginGroup ("transitionLibrary");

        if(args.size () <= args.indexOf ("--make-config") + 1)
        {
          std::cerr << "Please also specify a path containing lib/, statoil/ and user/!" << std::endl;
          throw std::runtime_error ("Please specify base path.");
        }

        QString pnetedir = args.at (args.indexOf ("--make-config") + 1);

        settings.setValue ("basePath", QString ("%1/lib").arg (pnetedir));

        //! \todo this should not be default!
        settings.beginWriteArray ("trustedPaths");
        settings.setArrayIndex (0);
        settings.setValue ("path", QString ("%1/statoil").arg (pnetedir));
        settings.endArray ();

        settings.beginWriteArray ("userPaths");
        settings.setArrayIndex (0);
        settings.setValue ("path", QString ("%1/user").arg (pnetedir));
        settings.endArray ();

        settings.endGroup ();
      }
    }

    void PetriNetEditor::createMainWindow ()
    {
      QString load;

      const QStringList& args (arguments ());
      if (args.contains ("--load"))
      {
        if(args.size () <= args.indexOf ("--load") + 1)
        {
          std::cerr << "--load requires a path" << std::endl;
          throw std::runtime_error ("Please specify filename.");
        }

        load = args.at (args.indexOf ("--load") + 1);
      }

      _mainWindow = new ui::MainWindow (load);
      _mainWindow->show ();
    }

    void PetriNetEditor::createTransitionLibrary ()
    {
      QSettings settings;

      settings.beginGroup ("transitionLibrary");

      if (!settings.contains ("basePath"))
      {
        //! \todo error message.
        std::cerr << "There is no base path set for the transition library." << std::endl;
        std::cerr << "Please run \"" << qPrintable (arguments ().at (0)) << " --make-config <path>\", where path contains lib." << std::endl;
        throw std::runtime_error ("Please configure first.");
      }

      _mainWindow->setTransitionLibraryPath (settings.value ("basePath").toString ());

      const int numTrusted (settings.beginReadArray ("trustedPaths"));
      for (int i (0); i < numTrusted; ++i)
      {
        settings.setArrayIndex (i);
        _mainWindow->addTransitionLibraryUserPath (settings.value ("path").toString (), true);
      }
      settings.endArray();

      const int numUser (settings.beginReadArray ("userPaths"));
      for (int i (0); i < numUser; ++i)
      {
        settings.setArrayIndex (i);
        _mainWindow->addTransitionLibraryUserPath (settings.value ("path").toString (), false);
      }
      settings.endArray ();

      settings.endGroup ();
    }
  }
}
