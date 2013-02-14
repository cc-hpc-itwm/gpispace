// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PENTE_HPP
#define _PENTE_HPP 1

#include <QApplication>
#include <QList>
#include <QTranslator>
#include <QSplashScreen>

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
      PetriNetEditor (int& argc, char *argv[]);
      virtual ~PetriNetEditor ();

      void startup ();

    private:
      QSplashScreen _splash;
      QTranslator _qtTranslator;
      QTranslator _penteTranslator;
      QList<ui::editor_window*> _editor_windows;

      void showSplashScreen();
      void setupLocalization();
      ui::editor_window* create_editor_window();
    };
  }
}

#endif
