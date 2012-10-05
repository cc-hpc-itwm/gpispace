// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_TRANSITION_HPP
#define _FHG_PNETE_UI_GRAPH_TRANSITION_HPP 1

#include <QPainterPath>
#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <QObject>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QAction;

#include <pnete/ui/graph/item.hpp>

#include <pnete/data/proxy.hpp>

#include <pnete/weaver/weaver.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class transition_item : public graph::item
        {
          Q_OBJECT;

        public:
          explicit transition_item ( ::xml::parse::type::transition_type& transition
                                   , ::xml::parse::type::net_type& net
                                   , graph::item* parent = NULL
                                   );

          const ::xml::parse::type::transition_type& transition() const;
          ::xml::parse::type::transition_type& transition();
          ::xml::parse::type::net_type& net();

          virtual QPainterPath shape() const;
          QRectF rectangle() const;

          const std::string& name() const;

          void repositionChildrenAndResize();

          void set_proxy (data::proxy::type*);
          data::proxy::type* proxy () const;

          enum { Type = transition_graph_type };
          virtual int type() const { return Type; }

          virtual void setPos (const QPointF&);

        protected:
          virtual void paint ( QPainter *painter
                             , const QStyleOptionGraphicsItem *option
                             , QWidget *widget
                             );

        private:
          //! \todo size verstellbar
          QSizeF _size;

          ::xml::parse::type::transition_type& _transition;
          ::xml::parse::type::net_type& _net;

          data::proxy::type* _proxy;
        };
      }
    }
  }
}

#endif
