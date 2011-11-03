#include <pnete/ui/graph/transition.hpp>

#include <QGraphicsScene>
#include <QPainter>
#include <QGraphicsSceneContextMenuEvent>
#include <QAction>
#include <QToolButton>
#include <QGraphicsProxyWidget>
#include <QPushButton>

#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/place.hpp>
// #include <pnete/ui/graph/cogwheel_button.hpp>
#include <pnete/ui/graph/connection.hpp>

#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/ui/graph/style/size.hpp>
#include <pnete/ui/graph/style/predicate.hpp>

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
          static boost::optional<const QColor&>
          color_if_name ( const QString& name
                        , const QColor& color
                        , const graph::item* i
                        )
          {
            if (style::predicate::is_transition (i))
              {
                const fhg::util::maybe<std::string>& n
                  (qgraphicsitem_cast<const item*>(i)->transition().name);

                if (n && *n == name.toStdString())
                  {
                    return boost::optional<const QColor&> (color);
                  }
              }

            return boost::none;
          }

          item::item ( ::xml::parse::type::transition_type& transition
                     , ::xml::parse::type::net_type& net
                     , graph::item* parent
                     )
            : graph::item (parent, &transition.prop)
            , _size (size::transition::width(), size::transition::height())
            , _transition (transition)
            , _net (net)
            , _proxy (NULL)
          {
            //            new cogwheel_button (this);
            setFlag (ItemIsSelectable);

            static const QColor border_color_normal (Qt::yellow);

            access_style().push<QColor>
              ( "border_color_normal"
              , mode::NORMAL
              , boost::bind (&color_if_name, "t", border_color_normal, _1)
              );

            static const QColor background_color (Qt::blue);

            access_style().push<QColor>
              ( "background_color"
              , mode::NORMAL
              , boost::bind (&color_if_name, "double", background_color, _1)
              );
          }

//           item::item ( const QString& filename
//                      , graph::item* parent
//                      )
//             : graph::item (parent)
//             , _size (size::transition::width(), size::transition::height())
//               //! \todo BIG UGLY FUCKING HACK EVIL DO NOT LOOK AT THIS BUT DELETE
//             , _transition(*static_cast<transition_type*> (malloc (sizeof (transition_type))))
//             , _proxy (NULL)
//           {
//             //! \todo WORK HERE, everything is missing
//           }

          const ::xml::parse::type::transition_type& item::transition () const
          {
            return _transition;
          }
          ::xml::parse::type::net_type& item::net ()
          {
            return _net;
          }

          void item::setPos (const QPointF& new_position)
          {
            const QPointF old_position (pos());

            graph::item::setPos (new_position);

            foreach (QGraphicsItem* collidingItem, collidingItems())
              {
                if (  qgraphicsitem_cast<item*> (collidingItem)
                   || qgraphicsitem_cast<place::item*> (collidingItem)
                   || qgraphicsitem_cast<port::top_level::item*> (collidingItem)
                   )
                  {
                    graph::item::setPos (old_position);

                    return;
                  }

                if ( port::item* port
                   = qgraphicsitem_cast<port::item*> (collidingItem)
                   )
                  {
                    if (port->parentItem() != this)
                      {
                        graph::item::setPos (old_position);

                        return;
                      }
                  }

              }
          }

          const std::string& item::name() const
          {
            return transition().name;
          }

          // void slot_change_name (QString name)
          // {
          //   internal()->change_manager().set_transition_name (reference(), name);
          // }

          void item::repositionChildrenAndResize()
          {
            const qreal padding (10.0); // hardcoded constant
            const qreal step (size::port::height());

            const QRectF bound (rectangle());
            const qreal top (bound.top());
            const qreal left (bound.left());
            const qreal right (bound.right());

            QPointF positionIn (left, top + padding);
            QPointF positionOut (right, top + padding);

            foreach (QGraphicsItem* child, childItems())
              {
                if (port::item* p = qgraphicsitem_cast<port::item*> (child))
                  {
                    if (p->direction() == connectable::direction::IN)
                      {
                        p->orientation (port::orientation::WEST);
                        p->setPos (style::raster::snap (positionIn));
                        positionIn.ry() += step + padding;
                      }
                    else
                      {
                        p->orientation (port::orientation::EAST);
                        p->setPos (style::raster::snap (positionOut));
                        positionOut.ry() += step + padding;
                      }
                  }
              }

            qreal& height (_size.rheight());
            height = qMax ( height
                          , qMax ( positionIn.y() - top
                                 , positionOut.y() - top
                                 )
                          );
            height = style::raster::snap (height);
          }

          void item::set_proxy (data::proxy::type* proxy_)
          {
            _proxy = proxy_;
          }
          data::proxy::type* item::proxy () const
          {
            return _proxy;
          }

          QRectF item::rectangle () const
          {
            const QSizeF half_size (_size / 2.0);
            return QRectF ( -half_size.width()
                          , -half_size.height()
                          , _size.width()
                          , _size.height()
                          );
          }
          QPainterPath item::shape () const
          {
            QPainterPath path;
            path.addRoundRect (rectangle (), 20); // hardcoded constant

            foreach (QGraphicsItem* child, childItems())
              {
                path = path.united (child->shape().translated (child->pos()));
              }

            return path;
          }

          void item::paint ( QPainter* painter
                           , const QStyleOptionGraphicsItem *option
                           , QWidget *widget
                           )
          {
            style::draw_shape (this, painter);

            painter->setPen (QPen (QBrush (Qt::black), 1.0));
            painter->setBackgroundMode (Qt::TransparentMode);

            QRectF rect (rectangle());
            rect.setWidth (rect.width() - size::port::width());
            rect.setHeight (rect.height() - size::port::width());
            rect.translate ( size::port::height() / 2.0
                           , size::port::height() / 2.0
                           );

            painter->drawText ( rect
                              , Qt::AlignCenter | Qt::TextWordWrap
                              , QString::fromStdString (name())
                              );
          }
        }
      }
    }
  }
}
