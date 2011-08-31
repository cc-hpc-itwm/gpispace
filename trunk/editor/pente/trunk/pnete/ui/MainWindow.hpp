#ifndef UIMAINWINDOW_HPP
#define UIMAINWINDOW_HPP

#include <QMainWindow>

class QAction;
class QDockWiget;
class QMenu;
class QMenuBar;
class QStatusBar;
class QToolBar;
class QTreeView;
class QVBoxLayout;
class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Scene;
    }
    namespace ui
    {
      class GraphView;
      class StructureView;

      class MainWindow : public QMainWindow
      {
        Q_OBJECT

      public:
        explicit MainWindow(QWidget *parent = NULL);

        void setTransitionLibraryPath(const QString& path);
        void addTransitionLibraryUserPath(const QString& path, bool trusted = false);

        graph::Scene * scene ();

      public slots:
        void save();
        void expandTree();

      private:
        QTreeView* _transitionLibrary;
        GraphView* _graphicsView;

        graph::Scene* _scene;

        void setupMenuAndToolbar();
        //! \todo This should be a complete class producing views for the same scene?
        void setupCentralWidget();
        void setupTransitionLibrary();

        StructureView* _structureView;
        void setupStructureView();
      };
    }
  }
}

#endif
