// bernd.loerwald@itwm.fraunhofer.de

#pragma once

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
        graph_view (graph::scene_type* scene, QWidget* parent = nullptr);

        graph::scene_type* scene() const;

        int zoom_level() const;

      public slots:
        void zoom (int to);

      signals:
        void zoomed (int to);

      protected:
        virtual void wheelEvent (QWheelEvent* event) override;

        virtual QSize sizeHint() const override;

      private:
        int _currentScale;
      };
    }
  }
}
