#ifndef UIMAINWINDOW_HPP
#define UIMAINWINDOW_HPP

#include <QMainWindow>

class QAction;
class QWidget;
class QVBoxLayout;
class QMenuBar;
class QMenu;
class QToolBar;
class QStatusBar;
class QDockWiget;
class QTreeView;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class GraphView;

      class MainWindow : public QMainWindow
      {
      Q_OBJECT
      
      public:
        explicit MainWindow(QWidget *parent = NULL);
        
        void setTransitionLibraryPath(const QString& path);
        void addTransitionLibraryUserPath(const QString& path);
      
      private:
        QTreeView* transitionLibrary;
        GraphView* graphicsView;
        
        void setupMenuAndToolbar();
        //! \todo This should be a complete class producing views for the same scene?
        void setupCentralWidget();
        void setupTransitionLibrary();
      };
    }
  }
}

#endif
