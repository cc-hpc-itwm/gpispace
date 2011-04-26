#include <QApplication>

#include "ui/MainWindow.hpp"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  
  fhg::pnete::ui::MainWindow w;
  //! \todo Decide where to put them.
  w.setTransitionLibraryPath(QCoreApplication::applicationDirPath() + "/../../demo/xml/");
  w.show();
  
  return a.exec();
}
