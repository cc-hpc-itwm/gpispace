// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_TRANSITION_HPP
#define _FHG_PNETE_UI_GRAPH_TRANSITION_HPP 1

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
        namespace transition
        {
          class item : public graph::item
          {
            Q_OBJECT;

          public:
            explicit item ( ::xml::parse::type::transition_type& transition
                          , ::xml::parse::type::net_type& net
                          , graph::item* parent = NULL
                          );
//             explicit item ( const QString& filename
//                           , graph::item* parent = NULL
//                           );

            const ::xml::parse::type::transition_type& transition() const;
            ::xml::parse::type::net_type& net();

            virtual QRectF boundingRect() const;
            virtual QPainterPath shape() const;

            QRectF bounding_rect (const QSizeF&) const;
            QPainterPath shape (const QSizeF&) const;

            const std::string& name() const;

            void repositionChildrenAndResize();

            void set_proxy (data::proxy::type*);
            data::proxy::type* proxy () const;

            enum { Type = transition_graph_type };
            virtual int type() const { return Type; }

            virtual void setPos (const QPointF&);

          public slots:
            void slot_delete(void);
            void slot_add_port(void);

          private slots:

          protected:
            virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
            virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

          private:
            //! \todo size verstellbar
            QSizeF _size;

            ::xml::parse::type::transition_type& _transition;
            ::xml::parse::type::net_type& _net;

            QMenu _menu_context;

            void init_menu_context();

            data::proxy::type* _proxy;
          };
        }
      }
    }
  }
}

#endif
