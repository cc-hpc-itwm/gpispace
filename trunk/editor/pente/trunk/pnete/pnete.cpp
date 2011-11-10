// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/pnete.hpp>

#include <set>
#include <vector>
#include <iostream>
#include <stdexcept>

#include <QSettings>
#include <QPixmap>
#include <QLocale>
#include <QLibraryInfo>

#include <pnete/ui/editor_window.hpp>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

namespace po = boost::program_options;

#define BE_PENTE 1

typedef std::string path_type;
typedef std::set<std::string> path_set_type;
typedef std::vector<std::string> path_list_type;

namespace library_transition
{
  namespace key
  {
#define CONST(_type,_name,_value)                       \
    static const _type& _name()                         \
    {                                                   \
      static const _type value (_value);                \
                                                        \
      return value;                                     \
    }

    CONST(QString, group, "library_transition");
    CONST(QString, path, "path");
    CONST(QString, paths_trusted, "paths_trusted");
    CONST(QString, paths_untrusted, "paths_untrusted");

#undef CONST
  }

  namespace detail
  {
    static path_set_type read (const QString& key)
    {
      path_set_type paths;

      QSettings settings;

      settings.beginGroup (key::group());

      const int size (settings.beginReadArray (key));

      for (int i (0); i < size; ++i)
        {
          settings.setArrayIndex (i);

          paths.insert (settings.value (key::path()).toString().toStdString());
        }

      settings.endArray();
      settings.endGroup();

      return paths;
    }

    static void write (const QString& key, const path_set_type& paths)
    {
      QSettings settings;

      settings.beginGroup (key::group());

      settings.beginWriteArray (key);

      int index (0);

      BOOST_FOREACH(const path_type& path, paths)
        {
          settings.setArrayIndex (index++);

          settings.setValue (key::path(), QString::fromStdString (path));
        }

      settings.endArray();
      settings.endGroup();
    }

    static void add ( fhg::pnete::ui::editor_window* editor_window
                    , const QString& key
                    , const bool trusted
                    )
    {
      QSettings settings;

      settings.beginGroup (key::group());

      const int size (settings.beginReadArray (key));

      for (int i (0); i < size; ++i)
      {
        settings.setArrayIndex (i);
        editor_window->add_transition_library_path
          (settings.value (key::path()).toString(), trusted);
      }
      settings.endArray();

      settings.endGroup ();
    }

  }

  static void update (const QString& key, const path_list_type& paths_new)
  {
    path_set_type paths_old (detail::read (key));

    paths_old.insert (paths_new.begin(), paths_new.end());

    detail::write (key, paths_old);
  }

  static void update (fhg::pnete::ui::editor_window* editor_window)
  {
    detail::add (editor_window, key::paths_trusted(), true);
    detail::add (editor_window, key::paths_untrusted(), false);
  }
}

int main (int argc, char *argv[])
{
  Q_INIT_RESOURCE (resources);

  QApplication::setApplicationName ("pnete");
  //! \todo Get SVN revision.
  QApplication::setApplicationVersion ("0.1");
  QApplication::setOrganizationDomain ("itwm.fhg.de");
  QApplication::setOrganizationName ("Fraunhofer ITWM");

  po::options_description desc ("options");

  std::string workflow ("-");
  path_list_type paths_trusted_new;
  path_list_type paths_untrusted_new;

  desc.add_options()
    ( "help,h", "this message")
    ( "workflow,w"
    , po::value<std::string>(&workflow)->default_value(workflow)
    , "workflow file name, - for stdin"
    )
    ( "lib-trusted,L"
    , po::value<path_list_type>(&paths_trusted_new)
    , "path to trusted library"
    )
    ( "lib-untrusted,T"
    , po::value<path_list_type>(&paths_untrusted_new)
    , "path to untrusted library"
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

  library_transition::update
    (library_transition::key::paths_trusted(), paths_trusted_new);
  library_transition::update
    (library_transition::key::paths_untrusted(), paths_untrusted_new);

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

      ui::editor_window* editor_window (create_editor_window());

      library_transition::update (editor_window);

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
