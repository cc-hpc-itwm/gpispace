// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_UI_GRAPH_VIEW_HPP
#define FHG_PNETE_UI_GRAPH_VIEW_HPP

#include <pnete/ui/graph_view.fwd.hpp>

#include <pnete/ui/graph/scene.fwd.hpp>

#include <QGraphicsView>
#include <QObject>

class QFocusEvent;
class QGraphicsScene;
class QWheelEvent;
class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class graph_view : public QGraphicsView
      {
        Q_OBJECT

        public:
        graph_view (graph::scene_type* scene, QWidget* parent = NULL);

        graph::scene_type* scene() const;

      public slots:
        void zoom (int to);

      signals:
        void zoomed (int to);

      protected:
        virtual void wheelEvent (QWheelEvent* event);

        virtual QSize sizeHint() const;

      private:
        int _currentScale;
      };
    }
  }
}

#endif
