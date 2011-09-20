#ifndef GRAPHTRANSITION_HPP
#define GRAPHTRANSITION_HPP 1

#include <QGraphicsItem>
#include <QPainterPath>
#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <QObject>
#include <QMenu>

class QPainter;
class QStyleOptionGraphicsItem;
class QGraphicsSceneContextMenuEvent;
class QWidget;
class QAction;
class QMenu;

#include <pnete/ui/graph_item.hpp>
#include <pnete/data/proxy.hpp>

#include <pnete/weaver/weaver.hpp>

#include <xml/parse/types.hpp>


namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Transition : public graph_item
      {
        Q_OBJECT
        Q_INTERFACES(QGraphicsItem)

        public:
          typedef ITVAL(XMLTYPE(net_type::transitions_type)) transition_type;
          typedef XMLTYPE(expression_type) expression_type;
          typedef XMLTYPE(mod_type) mod_type;

          explicit Transition( transition_type & data
                             , QGraphicsItem* parent = NULL
                             );
          explicit Transition ( const QString& filename
                              , QGraphicsItem* parent = NULL
                              );

          virtual QRectF boundingRect() const;
          virtual QPainterPath shape() const;

          const QString& name (const QString& name_);
          const QString& name() const;

          bool highlighted() const;

          void repositionChildrenAndResize();

        data::proxy::type* proxy (data::proxy::type*);
        data::proxy::type* proxy () const;

          enum
          {
            Type = TransitionType,
          };
          virtual int type() const
          {
            return Type;
          }

         public slots:
           void slot_delete(void);
           void slot_add_port(void);

         protected:
           virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

           virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
           virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
           virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
           virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
           virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
           virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

           //! \todo size verstellbar

           QPointF _dragStart;
           QSizeF _size;

           bool _highlighted;
           bool _dragging;

           transition_type & _data;

       private:
         QMenu _menu_context;

         void init_menu_context();

         QString _name;

        data::proxy::type * _proxy;
       };
     }
  }
}

#endif
