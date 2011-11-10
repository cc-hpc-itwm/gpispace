// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/pnete.hpp>

#include <iostream>
#include <stdexcept>

#include <QSettings>
#include <QPixmap>
#include <QLocale>
#include <QLibraryInfo>

#include <pnete/ui/editor_window.hpp>

#include <boost/program_options.hpp>

#include <iostream>

namespace po = boost::program_options;

#define BE_PENTE 1

int main (int argc, char *argv[])
{
  Q_INIT_RESOURCE (resources);

  po::options_description desc ("options");

  std::string workflow ("-");

  desc.add_options()
    ( "help,h", "this message")
    ( "workflow,w"
    , po::value<std::string>(&workflow)->default_value(workflow)
    , "workflow file name, - for stdin"
    )
    ;

  po::positional_options_description p;
  p.add("workflow", -1);

  po::variables_map vm;
  po::store( po::command_line_parser(argc, argv)
           . options(desc).positional(p).allow_unregistered().run()
           , vm
           );
  po::notify (vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;

      return EXIT_SUCCESS;
    }

  if (workflow == "-")
    {
      workflow = "/dev/stdin";
    }

  QApplication::setApplicationName ("pnete");
  //! \todo Get SVN revision.
  QApplication::setApplicationVersion ("0.1");
  QApplication::setOrganizationDomain ("itwm.fhg.de");
  QApplication::setOrganizationName ("Fraunhofer ITWM");

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
    PetriNetEditor::PetriNetEditor (int& argc, char *argv[])
    : QApplication (argc, argv)
    , _splash (QPixmap (":/pente.png"))
    , _editor_windows ()
    {}

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

      //! \note Fallback.
      _penteTranslator.load ("en_US", ":/localization/");
      installTranslator (&_penteTranslator);

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
      const int window_id (_editor_windows.count());
      _editor_windows.append (new ui::editor_window());
      _splash.close();
      _editor_windows.at (window_id)->show();
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

      _editor_windows.at (window_id)->set_transition_library_path
          (settings.value ("basePath").toString ());

      const int numTrusted (settings.beginReadArray ("trustedPaths"));
      for (int i (0); i < numTrusted; ++i)
      {
        settings.setArrayIndex (i);
        _editor_windows.at (window_id)->add_transition_library_user_path
            (settings.value ("path").toString (), true);
      }
      settings.endArray();

      const int numUser (settings.beginReadArray ("userPaths"));
      for (int i (0); i < numUser; ++i)
      {
        settings.setArrayIndex (i);
        _editor_windows.at (window_id)->add_transition_library_user_path
            (settings.value ("path").toString (), false);
      }
      settings.endArray ();

      settings.endGroup ();
    }
  }
}
