// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/transition.hpp>

// #include <pnete/ui/graph/cogwheel_button.hpp>
#include <pnete/ui/graph/connection.hpp>
#include <pnete/ui/graph/place.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/style/predicate.hpp>
#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/ui/graph/style/size.hpp>
#include <pnete/weaver/display.hpp>

#include <xml/parse/type/transition.hpp>

#include <QAction>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <QPushButton>
#include <QToolButton>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace
        {
          QBrush grad (const QColor& a, const QColor& b)
          {
            QLinearGradient g (QPointF(0.0, 0.0), QPointF (20.0, 20.0));
            g.setColorAt (0.0, a);
            g.setColorAt (1.0, b);
            return g;
          }
        }

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

          handle.connect_to_change_mgr
            (this, "transition_deleted", "data::handle::transition");

          handle.connect_to_change_mgr
            (this, "port_added", "data::handle::port");

          _style.push<QBrush> ( "background_brush"
                              , grad ( QColor::fromHsvF (0.0, 0.0, 0.98)
                                     , QColor::fromHsvF (0.0, 0.0, 0.93)
                                     )
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
              if (p->handle().is_input())
              {
                p->no_undo_setPos
                  (style::raster::snap (positionIn));
                positionIn.ry() += step + padding;
              }
              else if (p->handle().is_output())
              {
                p->no_undo_setPos
                  (style::raster::snap (positionOut));
                positionOut.ry() += step + padding;
              }
              else
              {
                //! \todo tunnel ports at bottom?
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

          painter->drawText ( rectangle().adjusted ( size::port::height()
                                                   , size::port::height()
                                                   , -size::port::height()
                                                   , -size::port::height()
                                                   )
                            , Qt::AlignCenter | Qt::TextWordWrap
                            , QString::fromStdString (name())
                            );
        }

        void transition_item::transition_deleted
          (const data::handle::transition& changed)
        {
          if (changed == handle())
          {
            scene()->removeItem (this);
            deleteLater();
          }
        }

        void transition_item::name_changed
          (const data::handle::transition& changed_handle, const QString&)
        {
          if (changed_handle == handle())
          {
            update();
          }
        }

        void transition_item::property_changed
          ( const data::handle::transition& changed_handle
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& value
          )
        {
          if (changed_handle == handle())
          {
            handle_property_change (key, value);
          }
        }

        void transition_item::port_added (const data::handle::port& port)
        {
          if (port.parent_is (handle().function()))
          {
            weaver::display::port (port, this);
          }
        }
      }
    }
  }
}