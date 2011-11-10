// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/pnete.hpp>

#include <iostream>
#include <stdexcept>

#include <QPixmap>
#include <QLocale>
#include <QLibraryInfo>

#include <pnete/setting.hpp>

#include <pnete/ui/editor_window.hpp>

#include <boost/program_options.hpp>

int main (int argc, char *argv[])
{
  namespace po = boost::program_options;
  namespace setting = fhg::pnete::setting;

  Q_INIT_RESOURCE (resources);

  setting::init();

  po::options_description desc ("options");

  std::string workflow ("-");
  setting::path_list_type paths_trusted_new;
  setting::path_list_type paths_untrusted_new;
  bool show_splash (setting::splash::show());

  desc.add_options()
    ( "help,h", "this message")
    ( "workflow,w"
    , po::value<std::string>(&workflow)->default_value(workflow)
    , "workflow file name, - for stdin"
    )
    ( "lib-trusted,L"
    , po::value<setting::path_list_type>(&paths_trusted_new)
    , "path to trusted library"
    )
    ( "lib-untrusted,T"
    , po::value<setting::path_list_type>(&paths_untrusted_new)
    , "path to untrusted library"
    )
    ( "show-splash,s"
    , po::value<bool>(&show_splash)->default_value(show_splash)
    , "show splash screen"
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

  setting::library_transition::update (paths_trusted_new, paths_untrusted_new);
  setting::splash::update (show_splash);

  try
  {
    fhg::pnete::PetriNetEditor pente (argc, argv);
    pente.startup ();

    return pente.exec ();
  }
  catch (const std::exception& e)
  {
    std::cerr << "EXCEPTION: " << e.what () << std::endl;

    return EXIT_FAILURE;
  }
}

namespace fhg
{
  namespace pnete
  {
    void PetriNetEditor::showSplashScreen ()
    {
      if (setting::splash::show())
        {
          _splash.show ();
          processEvents ();
        }
    }
    PetriNetEditor::PetriNetEditor (int& argc, char *argv[])
      : QApplication (argc, argv)
      , _splash (QPixmap (":/pente.png"))
      , _editor_windows ()
    {}

    void PetriNetEditor::startup ()
    {
      showSplashScreen ();

      setupLocalization ();

      ui::editor_window* editor_window (create_editor_window());

      setting::library_transition::update (editor_window);

      _splash.close();

      editor_window->show();
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

    void PetriNetEditor::setupLocalization()
    {
      const QString& locale (QLocale::system().name());

      _qtTranslator.load
        ( "qt_" + locale
        , QLibraryInfo::location(QLibraryInfo::TranslationsPath)
        );
      installTranslator (&_qtTranslator);

      //! \note Fallback.
      _penteTranslator.load ("en_US", ":/localization/");
      installTranslator (&_penteTranslator);

      _penteTranslator.load (locale, ":/localization/");
      installTranslator (&_penteTranslator);
    }

    ui::editor_window* PetriNetEditor::create_editor_window()
    {
      _editor_windows << new ui::editor_window();

      return _editor_windows.back();
    }
  }
}
