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
    private:
      QSplashScreen _splash;
      QTranslator _qtTranslator;
      QTranslator _penteTranslator;
      QList<ui::editor_window*> _editor_windows;

      void showSplashScreen ();

    public:
      PetriNetEditor (int & argc, char *argv[]);
      virtual ~PetriNetEditor ();

      void startup ();

      void setupLocalization ();
      void processCommandLine ();
      int create_editor_window ();
      void createTransitionLibrary (int window_id);
    };
  }
}

#endif
