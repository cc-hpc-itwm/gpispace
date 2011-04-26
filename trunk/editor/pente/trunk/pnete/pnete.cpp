#include <QApplication>

#include "ui/MainWindow.hpp"

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  
  fhg::pnete::ui::MainWindow w;
  //! \todo Get this dynamically.
  w.setTransitionLibraryPath("/Users/berndlorwald/Documents/Arbeit/SDPA/svn/trunk/editor/pente/trunk/demo/xml/");
  w.show();
  
  return a.exec();
}
