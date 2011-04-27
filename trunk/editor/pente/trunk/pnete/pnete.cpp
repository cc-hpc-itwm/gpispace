#include <QApplication>

#include "ui/MainWindow.hpp"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  
  fhg::pnete::ui::MainWindow w;
  //! \todo Decide where to put them.
  w.setTransitionLibraryPath(QCoreApplication::applicationDirPath() + "/../../demo/lib/");
  //! \todo Get from some config.
  w.addTransitionLibraryUserPath(QCoreApplication::applicationDirPath() + "/../../demo/statoil/");
  w.addTransitionLibraryUserPath(QCoreApplication::applicationDirPath() + "/../../demo/user/");
  w.show();
  
  return a.exec();
}
