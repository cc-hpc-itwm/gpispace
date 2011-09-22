// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_PLACE_HPP
#define _FHG_PNETE_UI_GRAPH_PLACE_HPP 1

#include <QPainter>
#include <QRectF>
#include <QStaticText>

class QWidget;
class QStyleOptionGraphicsItem;

#include <pnete/ui/graph/connectable_item.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class place : public connectable_item
        {
          Q_OBJECT;

        public:
          place (item* parent = NULL);
          const QString& name (const QString& name_);

        public slots:
          void refresh_content();

        public:
          virtual QRectF boundingRect() const;
          virtual void paint ( QPainter* painter
                             , const QStyleOptionGraphicsItem* option
                             , QWidget* widget = NULL
                             );
          virtual void mouseMoveEvent (QGraphicsSceneMouseEvent* event);
          virtual void mousePressEvent (QGraphicsSceneMouseEvent* event);
          virtual void mouseReleaseEvent (QGraphicsSceneMouseEvent* event);

          enum { Type = place_graph_type };
          virtual int type() const { return Type; }

        private:
          QStaticText _content;

          bool _dragging;
          QPointF _drag_start;

          QString _name;
        };
      }
    }
  }
}

#endif
