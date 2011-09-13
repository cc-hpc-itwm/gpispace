// bernd.loerwald@itwm.fraunhofer.de

#include "pnete.hpp"

#include <iostream>
#include <stdexcept>

#include <QSettings>
#include <QPixmap>
#include <QLocale>
#include <QLibraryInfo>

#include "ui/editor_window.hpp"

#define BE_PENTE 1

int main (int argc, char *argv[])
{
  Q_INIT_RESOURCE (resources);

  try
  {
    fhg::pnete::PetriNetEditor pente (argc, argv);
    pente.startup ();
    return pente.exec ();
  }
  catch (const std::exception& e)
  {
    std::cerr << "EXCEPTION: " << e.what () << std::endl;
    return -1;
  }
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
    PetriNetEditor::PetriNetEditor (int & argc, char *argv[])
    : QApplication (argc, argv)
    , _splash (QPixmap (":/pente.png"))
    , _editor_windows ()
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
      int window_id (create_editor_window ());
      createTransitionLibrary (window_id);
    }

    PetriNetEditor::~PetriNetEditor ()
    {
      foreach (ui::editor_window* w, _editor_windows)
      {
        w->hide();
        delete w;
      }
      _editor_windows.clear();
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

    int PetriNetEditor::create_editor_window ()
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

      int window_id (_editor_windows.count());
      _editor_windows.append (new ui::editor_window (load));
      _splash.close();
      _editor_windows.at (window_id)->show ();
      return window_id;
    }

    void PetriNetEditor::createTransitionLibrary (int window_id)
    {
      QSettings settings;

      settings.beginGroup ("transitionLibrary");

      if (!settings.contains ("basePath"))
      {
        //! \todo error message.
        std::cerr << "There is no base path set for the transition library.\n"
                  << "Please run \"" << qPrintable (arguments ().at (0))
                  << " --make-config <path>\", where path contains lib.\n";
        throw std::runtime_error ("Please configure first.");
      }

      _editor_windows.at (window_id)->setTransitionLibraryPath
          (settings.value ("basePath").toString ());

      const int numTrusted (settings.beginReadArray ("trustedPaths"));
      for (int i (0); i < numTrusted; ++i)
      {
        settings.setArrayIndex (i);
        _editor_windows.at (window_id)->addTransitionLibraryUserPath
            (settings.value ("path").toString (), true);
      }
      settings.endArray();

      const int numUser (settings.beginReadArray ("userPaths"));
      for (int i (0); i < numUser; ++i)
      {
        settings.setArrayIndex (i);
        _editor_windows.at (window_id)->addTransitionLibraryUserPath
            (settings.value ("path").toString (), false);
      }
      settings.endArray ();

      settings.endGroup ();
    }
  }
}
