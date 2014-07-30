// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PENTE_HPP
#define _PENTE_HPP 1

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

      void startup();
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
      ui::editor_window* create_editor_window();
    };
  }
}

#endif
