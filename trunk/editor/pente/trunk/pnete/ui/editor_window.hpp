// bernd.loerwald@itwm.fraunhofer.de

#ifndef UI_EDITOR_WINDOW_HPP
#define UI_EDITOR_WINDOW_HPP 1

#include <QMainWindow>
#include <QObject>

class QString;
class QTreeView;
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
      class view_manager;
      class StructureView;
      class GraphView;

      class editor_window : public QMainWindow
      {
        Q_OBJECT

        public:
          explicit editor_window ( const QString& load = ""
                                 , QWidget *parent = NULL
                                 );

          void setTransitionLibraryPath (const QString& path);
          void addTransitionLibraryUserPath ( const QString& path
                                            , bool trusted = false
                                            );

        public slots:
          void create();
          void save();
          void open();
          void close_tab();
          void quit();
          void expandTree();

          void scene_changed (graph::Scene*);
          void view_changed (GraphView*);

        private:
          QTreeView* _transitionLibrary;
          view_manager* _view_manager;

          void setupMenuAndToolbar();
          void setupTransitionLibrary();

          StructureView* _structureView;
          void setupStructureView (const QString& load);
      };
    }
  }
}

#endif
