// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_PLACE_HPP
#define _FHG_PNETE_UI_GRAPH_PLACE_HPP 1

#include <QPainter>
#include <QRectF>
#include <QStaticText>

#include <boost/optional.hpp>

#include <xml/parse/types.hpp>

#include <pnete/weaver/weaver.hpp>

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
        namespace place
        {
          class item : public graph::connectable::item
          {
            Q_OBJECT;

          public:
            typedef ITVAL(XMLTYPE(net_type::places_type)) place_type;

            item
            ( place_type& place
            , boost::optional< ::xml::parse::type::type_map_type&> type_map
            = boost::none
            , item* parent = NULL
            );

            const QString& name (const QString& name_);
            const QString& name() const;

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
            place_type& _place;

            QStaticText _content;

            bool _dragging;
            QPointF _drag_start;

            QString _name;
          };
        }
      }
    }
  }
}

#endif
