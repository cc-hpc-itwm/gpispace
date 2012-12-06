// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/place.hpp>

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>

#include <pnete/ui/graph/style/raster.hpp>

#include <pnete/ui/graph/association.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/transition.hpp>

#include <xml/parse/type/place.hpp>

#include <QPainter>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        place_item::place_item
          ( const data::handle::place& handle
          , base_item* parent
          )
            : connectable_item ( connectable::direction::BOTH
                               , parent
                               )
            , _handle (handle)
            , _content()
        {
          refresh_content();

          handle.connect_to_change_mgr
            ( this
            , "property_changed"
            , "  const data::handle::place&"
              ", const ::we::type::property::key_type&"
              ", const ::we::type::property::value_type&"
            );

          handle.connect_to_change_mgr
            ( this
            , "place_type_set", "type_changed"
            , "const data::handle::place&, const QString&"
            );

          connect ( this, SIGNAL (association_added (association*))
                  , this, SLOT (slot_association_added (association*))
                  );
          connect ( this, SIGNAL (association_removed (association*))
                  , this, SLOT (slot_association_removed (association*))
                  );
          connect ( this, SIGNAL (association_added (association*))
                  , this, SLOT (association_changed_in_any_way())
                  );
          connect ( this, SIGNAL (association_removed (association*))
                  , this, SLOT (association_changed_in_any_way())
                  );
        }

        const data::handle::place& place_item::handle() const
        {
          return _handle;
        }

        const std::string& place_item::we_type() const
        {
          return connectable_item::we_type (handle().get().type);
        }

        std::string place_item::name() const
        {
          return handle().get().name();
        }

        void place_item::refresh_content()
        {
          _content.setText ( QString::fromStdString (name())
                           + " :: "
                           + QString::fromStdString (we_type())
                           );
        }

        const QStaticText& place_item::content() const
        {
          return _content;
        }
        QSizeF place_item::content_size() const
        {
          return content().size();
        }
        QPointF place_item::content_pos() const
        {
          const QSizeF half_size (content_size() / 2.0);

          return QPointF (-half_size.width(), -half_size.height());
        }

        QPainterPath place_item::shape () const
        {
          QPainterPath path;
          if (!handle().is_implicit())
          {
            static const qreal d (3.0);

            path.addRoundRect ( QRectF
                                ( content_pos() - QPointF (d, d)
                                , content_size() + QSizeF (2 * d, 2 * d)
                                )
                              , 2 * d
                              , 2 * d
                              );
          }
          return path;
        }

        void place_item::paint ( QPainter* painter
                               , const QStyleOptionGraphicsItem* option
                               , QWidget* widget
                               )
        {
          if (!handle().is_implicit())
          {
            style::draw_shape (this, painter);

            painter->drawStaticText (content_pos(), content());
          }
        }

        void place_item::setPos (const QPointF& new_position)
        {
          const QPointF old_position (pos());

          base_item::setPos (new_position);

          foreach (QGraphicsItem* collidingItem, collidingItems())
          {
            if (  qgraphicsitem_cast<place_item*> (collidingItem)
               || qgraphicsitem_cast<transition_item*> (collidingItem)
               || qgraphicsitem_cast<top_level_port_item*> (collidingItem)
               )
            {
              base_item::setPos (old_position);

              return;
            }
          }
        }

        void place_item::type_changed
          ( const QObject* origin
          , const data::handle::place& changed_handle
          , const QString&
          )
        {
          if (changed_handle == handle())
          {
            refresh_content();
            update();
          }
        }

        void place_item::property_changed
          ( const QObject* origin
          , const data::handle::place& changed_handle
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& value
          )
        {
          if (origin != this && changed_handle == handle())
          {
            handle_property_change (key, value);
          }
        }

        void place_item::slot_association_added (association* c)
        {
          connectable_item* other_side
            (c->start() != this ? c->start() : c->end());
          //! \note port
          connect ( other_side, SIGNAL (scaleChanged())
                  , this, SLOT (association_changed_in_any_way())
                  );
          connect ( other_side, SIGNAL (xChanged())
                  , this, SLOT (association_changed_in_any_way())
                  );
          connect ( other_side, SIGNAL (yChanged())
                  , this, SLOT (association_changed_in_any_way())
                  );
          connect ( other_side, SIGNAL (zChanged())
                  , this, SLOT (association_changed_in_any_way())
                  );
          //! \note transition
          for ( QGraphicsObject* parent = other_side->parentObject()
              ; parent
              ; parent = parent->parentObject()
              )
          {
            connect ( parent, SIGNAL (scaleChanged())
                    , this, SLOT (association_changed_in_any_way())
                    );
            connect ( parent, SIGNAL (xChanged())
                    , this, SLOT (association_changed_in_any_way())
                    );
            connect ( parent, SIGNAL (yChanged())
                    , this, SLOT (association_changed_in_any_way())
                    );
            connect ( parent, SIGNAL (zChanged())
                    , this, SLOT (association_changed_in_any_way())
                    );
          }
        }
        void place_item::slot_association_removed (association* c)
        {
          connectable_item* other_side
            (c->start() != this ? c->start() : c->end());
          other_side->disconnect (this);
          for ( QGraphicsObject* parent = other_side->parentObject()
              ; parent
              ; parent = parent->parentObject()
              )
          {
            parent->disconnect (this);
          }
        }

        void place_item::association_changed_in_any_way()
        {
          if (handle().is_implicit())
          {
            QPointF center (0.0, 0.0);
            int points (0);
            foreach (association* item, associations())
            {
              center += item->start() != this
                      ? item->start()->scenePos()
                      : item->end()->scenePos();
              ++points;
            }
            no_undo_no_raster_setPos (center / qreal (points));
          }
        }


//           void place_item::mouseMoveEvent (QGraphicsSceneMouseEvent* event)
//           {
//             if (mode() == style::mode::DRAG)
//               {
//                 setPos (style::raster::snap (pos() + event->pos() - _drag_start));
//                 event->accept();
//                 scene()->update();
//               }
//             else
//               {
//                 connectable_item::mouseMoveEvent (event);
//               }
//           }
//           void place_item::mousePressEvent (QGraphicsSceneMouseEvent* event)
//           {
//             if (event->modifiers() == Qt::ControlModifier)
//               {
//                 mode_push (style::mode::DRAG);
//                 _drag_start = event->pos();
//                 event->accept();
//                 return;
//               }

//             connectable_item::mousePressEvent (event);
//           }
      }
    }
  }
}
