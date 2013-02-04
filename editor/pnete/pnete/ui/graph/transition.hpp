// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef FHG_PNETE_UI_GRAPH_TRANSITION_HPP
#define FHG_PNETE_UI_GRAPH_TRANSITION_HPP

#include <pnete/ui/graph/transition.fwd.hpp>

#include <pnete/data/handle/transition.hpp>
#include <pnete/ui/graph/base_item.hpp>

#include <QPainterPath>
#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <QObject>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QAction;

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

          enum { Type = transition_graph_type };
          virtual int type() const { return Type; }

          virtual void setPos (const QPointF&);

        public slots:
          void transition_deleted (const data::handle::transition&);

          void property_changed
            ( const data::handle::transition&
            , const we::type::property::key_type&
            , const we::type::property::value_type&
            );

          void name_changed (const data::handle::transition&, const QString&);

        protected:
          virtual void paint ( QPainter *painter
                             , const QStyleOptionGraphicsItem *option
                             , QWidget *widget
                             );

        private:
          //! \todo size verstellbar
          QSizeF _size;

          data::handle::transition _handle;
        };
      }
    }
  }
}

#endif
