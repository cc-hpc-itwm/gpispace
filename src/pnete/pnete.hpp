// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <pnete/data/manager.hpp>

#include <fhg/util/dl.hpp>

#include <QApplication>
#include <QList>
#include <QTranslator>
#include <QSplashScreen>

#include <list>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class editor_window;
    }

    class PetriNetEditor : public QApplication
    {
    public:
      PetriNetEditor
        (std::list<util::scoped_dlhandle> const& plugins, int& argc, char *argv[]);
      virtual ~PetriNetEditor ();

      void startup ( boost::optional<std::string> orchestrator_host
                   , boost::optional<unsigned short> orchestrator_port
                   );
      void logging (int port);
      void exec_monitor (int port);

    private:
      QSplashScreen _splash;
      QTranslator _qtTranslator;
      QTranslator _penteTranslator;
      data::manager _data_manager;
      std::list<util::scoped_dlhandle> const& _plugins;
      QList<ui::editor_window*> _editor_windows;

      void showSplashScreen();
      void setupLocalization();
      ui::editor_window* create_editor_window
        ( boost::optional<std::string> orchestrator_host
        , boost::optional<unsigned short> orchestrator_port
        );
    };
  }
}
