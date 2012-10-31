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

#include <pnete/ui/graph/base_item.hpp>

#include <pnete/data/handle/transition.hpp>

#include <pnete/data/proxy.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class transition_item : public base_item
        {
          Q_OBJECT;

        public:
          explicit transition_item ( const data::handle::transition& handle
                                   , base_item* parent = NULL
                                   );

          virtual const data::handle::transition& handle() const;

          virtual QPainterPath shape() const;
          QRectF rectangle() const;

          std::string name() const;

          void repositionChildrenAndResize();

          void set_proxy (data::proxy::type*);
          data::proxy::type* proxy () const;

          enum { Type = transition_graph_type };
          virtual int type() const { return Type; }

          virtual void setPos (const QPointF&);

        public slots:
          void property_changed
            ( const QObject* origin
            , const data::handle::transition& changed_handle
            , const ::we::type::property::key_type& key
            , const ::we::type::property::value_type& from
            , const ::we::type::property::value_type& to
            );

        protected:
          virtual void paint ( QPainter *painter
                             , const QStyleOptionGraphicsItem *option
                             , QWidget *widget
                             );

        private:
          //! \todo size verstellbar
          QSizeF _size;

          data::handle::transition _handle;

          data::proxy::type* _proxy;
        };
      }
    }
  }
}

#endif
