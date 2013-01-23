// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

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

#include <xml/parse/type/transition.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        transition_item::transition_item
          ( const data::handle::transition& handle
          , base_item* parent
          )
            : base_item (parent)
            , _size (size::transition::width(), size::transition::height())
            , _handle (handle)
        {
          //            new cogwheel_button (this);
          setFlag (ItemIsSelectable);

          handle.connect_to_change_mgr
            ( this
            , "name_set", "name_changed"
            , "data::handle::transition, QString"
            );

          handle.connect_to_change_mgr
            ( this
            , "property_changed"
            , "data::handle::transition, "
              "we::type::property::key_type, we::type::property::value_type"
            );
        }

        const data::handle::transition& transition_item::handle() const
        {
          return _handle;
        }

        void transition_item::setPos (const QPointF& new_position)
        {
          const QPointF old_position (pos());

          base_item::setPos (new_position);

          foreach (QGraphicsItem* collidingItem, collidingItems())
          {
            if (  qgraphicsitem_cast<transition_item*> (collidingItem)
               || qgraphicsitem_cast<place_item*> (collidingItem)
               || qgraphicsitem_cast<top_level_port_item*> (collidingItem)
               )
            {
              base_item::setPos (old_position);

              return;
            }

            if ( port_item* port
               = qgraphicsitem_cast<port_item*> (collidingItem)
               )
            {
              if (port->parentItem() != this)
              {
                base_item::setPos (old_position);

                return;
              }
            }

          }
        }

        std::string transition_item::name() const
        {
          return handle().get().name();
        }

        void transition_item::repositionChildrenAndResize()
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
            if (port_item* p = qgraphicsitem_cast<port_item*> (child))
            {
              if (p->handle().get().direction() == we::type::PORT_IN)
              {
                p->no_undo_setPos
                  (style::raster::snap (positionIn));
                positionIn.ry() += step + padding;
              }
              else
              {
                p->no_undo_setPos
                  (style::raster::snap (positionOut));
                positionOut.ry() += step + padding;
              }
            }
          }

          _size.rheight()
            = style::raster::snap ( qMax ( _size.rheight()
                                         , qMax ( positionIn.y() - top
                                                , positionOut.y() - top
                                                )
                                         )
                                  );
        }

        QRectF transition_item::rectangle () const
        {
          const QSizeF half_size (_size / 2.0);
          return QRectF ( -half_size.width()
                        , -half_size.height()
                        , _size.width()
                        , _size.height()
                        );
        }
        QPainterPath transition_item::shape () const
        {
          QPainterPath path;
          path.addRoundRect (rectangle (), 20); // hardcoded constant

          foreach (QGraphicsItem* child, childItems())
          {
            path = path.united (child->shape().translated (child->pos()));
          }

          return path;
        }

        void transition_item::paint ( QPainter* painter
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

        void transition_item::name_changed
          ( const QObject* origin
          , const data::handle::transition& changed_handle
          , const QString&
          )
        {
          if (changed_handle == handle())
          {
            repositionChildrenAndResize();
            update();
          }
        }

        void transition_item::property_changed
          ( const QObject* origin
          , const data::handle::transition& changed_handle
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& value
          )
        {
          if (origin != this && changed_handle == handle())
          {
            handle_property_change (key, value);
          }
        }
      }
    }
  }
}
