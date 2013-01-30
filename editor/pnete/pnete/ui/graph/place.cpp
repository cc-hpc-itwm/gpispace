// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/place.hpp>

#include <pnete/ui/graph/association.hpp>
#include <pnete/ui/graph/port.hpp>
#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/ui/graph/transition.hpp>

#include <util/qt/cast.hpp>
#include <util/qt/scoped_property_setter.hpp>

#include <xml/parse/type/place.hpp>

#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

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
          boost::optional<const Qt::PenStyle&> pen_style (const base_item* item)
          {
            if ( fhg::util::qt::throwing_qobject_cast<const place_item*>
                 (item)->handle().is_virtual()
               )
            {
              static Qt::PenStyle why_is_the_return_value_a_reference
                (Qt::DashLine);

              return why_is_the_return_value_a_reference;
            }

            return boost::none;
          }
        }

        place_item::place_item ( const data::handle::place& handle
                               , base_item* parent
                               )
          : connectable_item (parent)
          , _handle (handle)
          , _content()
        {
          refresh_content();

          handle.connect_to_change_mgr
            ( this
            , "property_changed"
            , "data::handle::place, "
              "we::type::property::key_type, we::type::property::value_type"
            );

          handle.connect_to_change_mgr
            ( this
            , "name_set", "type_or_name_changed"
            , "data::handle::place, QString"
            );
          handle.connect_to_change_mgr
            ( this
            , "type_set", "type_or_name_changed"
            , "data::handle::place, QString"
            );

          handle.connect_to_change_mgr
            (this, "place_is_virtual_changed", "data::handle::place, bool");

          handle.connect_to_change_mgr
            (this, "place_deleted", "data::handle::place");

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

          update_implicity();

          _style.push<Qt::PenStyle> ("border_style", mode::NORMAL, pen_style);
        }

        const data::handle::place& place_item::handle() const
        {
          return _handle;
        }

        bool place_item::is_connectable_with (const connectable_item* i) const
        {
          //! \note Places connect to everything except for places.
          return qobject_cast<const place_item*> (i) == NULL
            && connectable_item::is_connectable_with (i);
        }

        const std::string& place_item::we_type() const
        {
          return connectable_item::we_type (handle().get().type());
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

          static const qreal d (3.0);
          path.addRoundRect ( QRectF
                              ( content_pos() - QPointF (d, d)
                              , content_size() + QSizeF (2 * d, 2 * d)
                              )
                            , 2 * d
                            , 2 * d
                            );

          return path;
        }

        void place_item::paint ( QPainter* painter
                               , const QStyleOptionGraphicsItem* option
                               , QWidget* widget
                               )
        {
          painter->setOpacity (mode() == mode::HIGHLIGHT ? 1.0 : opacity());

          style::draw_shape (this, painter);

          painter->drawStaticText (content_pos(), content());
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

        void place_item::place_deleted ( const QObject* origin
                                       , const data::handle::place& changed
                                       )
        {
          if (changed == handle())
          {
            scene()->removeItem (this);
            deleteLater();
          }
        }

        void place_item::type_or_name_changed
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

          update_implicity();
        }

        void place_item::update_implicity()
        {
          if (handle().is_implicit())
          {
            setOpacity (0.2);
          }
          else
          {
            setOpacity (1.0);
          }
        }

        void place_item::place_is_virtual_changed
          (const QObject*, const data::handle::place& changed_handle, bool)
        {
          if (changed_handle == handle())
          {
            clear_style_cache();
            update();
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

        bool place_item::is_movable() const
        {
          return !handle().is_implicit();
        }
      }
    }
  }
}
