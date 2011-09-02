// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PENTE_HPP
#define _PENTE_HPP 1

#include <QApplication>
#include <QTranslator>
#include <QSplashScreen>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class MainWindow;
    }

    class PetriNetEditor : public QApplication
    {
    private:
      QSplashScreen _splash;
      QTranslator _qtTranslator;
      QTranslator _penteTranslator;
      ui::MainWindow* _mainWindow;

      void showSplashScreen ();
      void hideSplashScreen ();

    public:
      PetriNetEditor (int & argc, char *argv[]);
      virtual ~PetriNetEditor ();

      void startup ();

      void setupLocalization ();
      void processCommandLine ();
      void createMainWindow ();
      void createTransitionLibrary ();
    };
  }
}

#endif
