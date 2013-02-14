// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_UI_GRAPH_PENDING_CONNECTION_HPP
#define FHG_PNETE_UI_GRAPH_PENDING_CONNECTION_HPP

#include <pnete/ui/graph/pending_connection.fwd.hpp>

#include <pnete/ui/graph/base_item.hpp>
#include <pnete/ui/graph/connectable_item.fwd.hpp>

#include <QPointF>

class QPainter;
class QPainterPath;
class QStyleOptionGraphicsItem;
class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class pending_connection : public base_item
        {
        public:
          pending_connection ( const connectable_item* fixed_end
                             , const QPointF& open_end
                             , base_item* parent = NULL
                             );

          void open_end (const QPointF&);
          const connectable_item* fixed_end() const;

          virtual QPainterPath shape() const;
          virtual void paint
            (QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

          enum { Type = pending_connection_graph_type };
          virtual int type() const { return Type; }

        private:
          const connectable_item* _fixed_end;
          QPointF _open_end;
        };
      }
    }
  }
}

#endif
