// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/port.hpp>

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <QStaticText>
#include <QDebug>
#include <QMenu>
#include <QPointF>

#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/connection.hpp>

#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/ui/graph/style/size.hpp>
#include <pnete/ui/graph/style/cap.hpp>
#include <pnete/ui/graph/style/predicate.hpp>

#include <xml/parse/type/port.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        port_item::port_item
          ( const data::handle::port& handle
          , transition_item* parent
          )
            : connectable_item ( handle.get().direction() == we::type::PORT_IN
                               ? connectable::direction::IN
                               : connectable::direction::OUT
                               , parent
                               )
            , _handle (handle)
            , _length (size::port::width())
        {
          handle.connect_to_change_mgr
            ( this
            , "property_changed"
            , "data::handle::port, "
              "we::type::property::key_type, we::type::property::value_type"
            );

          handle.connect_to_change_mgr
            ( this
            , "name_set", "type_or_name_changed"
            , "data::handle::port, QString"
            );
          handle.connect_to_change_mgr
            ( this
            , "type_set", "type_or_name_changed"
            , "data::handle::port, QString"
            );


          //            setAcceptHoverEvents (true);
          //! \todo verbose name

          refresh_content();
        }

        const data::handle::port& port_item::handle() const
        {
          return _handle;
        }

        void port_item::slot_set_type ()
        {
          qDebug() << "port::slot_set_type()";
        }

        void port_item::setPos_no_collision_detection (const QPointF& new_position)
        {
          connectable_item::setPos (new_position);
        }

        void port_item::setPos (const QPointF& new_position)
        {
          const QPointF old_position (pos());

          connectable_item::setPos (fitting_position (new_position));

          // do not move, when now colliding with a different port
          foreach (QGraphicsItem* collidingItem, collidingItems())
          {
            if ( qgraphicsitem_cast<port_item*>(collidingItem)
               && collidingItem->parentItem() == parentItem()
               )
            {
              connectable_item::setPos (old_position);

              return;
            }

            if ( transition_item* transition
               = qgraphicsitem_cast<transition_item*> (collidingItem)
               )
            {
              if (parentItem() != transition)
              {
                connectable_item::setPos (old_position);

                return;
              }
            }
          }
        }

        namespace
        {
          bool is_opposite_type_or_direction
            (const port_item* lhs, const connectable_item* rhs)
          {
            if (qobject_cast<const port_item*> (rhs))
            {
              const bool lhs_is_top_level
                (qobject_cast<const top_level_port_item*> (lhs));
              const bool rhs_is_top_level
                (qobject_cast<const top_level_port_item*> (rhs));
              const bool both_top_level (lhs_is_top_level && rhs_is_top_level);
              const bool both_non_top_level
                (!lhs_is_top_level && !rhs_is_top_level);
              const bool same_level (both_top_level || both_non_top_level);
              const bool same_direction (lhs->direction() == rhs->direction());

              return same_level ? !same_direction : same_direction;
            }
            else
            {
              return true;
            }
          }
        }

        bool port_item::is_connectable_with (const connectable_item* item) const
        {
          //! \note Only allow one connection.
          return _associations.isEmpty()
            && is_opposite_type_or_direction (this, item)
            && connectable_item::is_connectable_with (item);
        }

        static qreal quad (const qreal x)
        {
          return x * x;
        }

        QPointF port_item::fitting_position (QPointF position)
        {
          if ( const transition_item* transition
             = qgraphicsitem_cast<const transition_item*> (parentItem())
             )
          {
            const QPointF minimum_distance
              ( boundingRect().width() / 2.0 + 1.0    // hardcoded constants
              , boundingRect().height() / 2.0 + 1.0   // hardcoded constants
              );

            const QRectF bounding (transition->rectangle());

            const qreal to_left (quad (position.x() - bounding.left()));
            const qreal to_right (quad (position.x() - bounding.right()));
            const qreal to_top (quad (position.y() - bounding.top()));
            const qreal to_bottom (quad (position.y() - bounding.bottom()));

            port::orientation::type orientation
              ( qMin (to_top, to_bottom) < qMin (to_left, to_right)
              ? ( to_top < to_bottom
                ? port::orientation::NORTH
                : port::orientation::SOUTH
                )
              : ( to_left < to_right
                ? port::orientation::WEST
                : port::orientation::EAST
                )
              );

            if(  orientation == port::orientation::WEST
              || orientation == port::orientation::EAST
              )
            {
              position.setX ( orientation == port::orientation::WEST
                            ? bounding.left()
                            : bounding.right()
                            );
              position.setY ( qBound ( bounding.top() + minimum_distance.y()
                                     , position.y()
                                     , bounding.bottom() - minimum_distance.y()
                                     )
                            );
            }
            else
            {
              position.setX ( qBound ( bounding.left() + minimum_distance.x()
                                     , position.x()
                                     , bounding.right() - minimum_distance.x()
                                     )
                            );
              position.setY ( orientation == port::orientation::NORTH
                            ? bounding.top()
                            : bounding.bottom()
                            );
            }
          }

          return style::raster::snap (position);
        }

        const qreal& port_item::length() const
        {
          return _length;
        }

        const std::string& port_item::name() const
        {
          return handle().get().name();
        }
        const std::string& port_item::we_type() const
        {
          return connectable_item::we_type (handle().get().type());
        }

        port::orientation::type port_item::orientation() const
        {
          if ( const transition_item* transition
             = qgraphicsitem_cast<const transition_item*> (parentItem())
             )
          {
            const QRectF bounding (transition->rectangle());

            const qreal to_left (quad (pos().x() - bounding.left()));
            const qreal to_right (quad (pos().x() - bounding.right()));
            const qreal to_top (quad (pos().y() - bounding.top()));
            const qreal to_bottom (quad (pos().y() - bounding.bottom()));

            return
              ( qMin (to_top, to_bottom) < qMin (to_left, to_right)
              ? ( to_top < to_bottom
                ? port::orientation::NORTH
                : port::orientation::SOUTH
                )
              : ( to_left < to_right
                ? port::orientation::WEST
                : port::orientation::EAST
                )
              );
          }
          //! \note Top level port
          else
          {
            return handle().get().direction() == we::type::PORT_IN
              ? port::orientation::EAST : port::orientation::WEST;
          }
        }

        // void port_item::delete_connection()
        // {
        //   if(_connection)
        //   {
        //     class connection* const backup (_connection);
        //     const QRectF area (backup->boundingRect());
        //     backup->start (NULL);
        //     backup->end (NULL);
        //     delete backup;
        //     scene()->update (area);
        //   }
        // }

        void port_item::refresh_content()
        {
          setToolTip (QString::fromStdString (name()) + " :: " + QString::fromStdString (we_type()));

          _length = qMax( _length
                        , QStaticText(QString::fromStdString (name())).size().width()
                        + 2 * size::cap::length()
                        );

        }

        static qreal angle (const port::orientation::type& o)
        {
          switch (o)
          {
          case port::orientation::NORTH: return -90.0;
          case port::orientation::EAST: return 0.0;
          case port::orientation::SOUTH: return 90.0;
          case port::orientation::WEST: return 180.0;
          default:
            throw std::runtime_error ("angle (NON_ORIENTATION)!?");
          }
        }

        void port_item::add_cap_for_direction ( QPolygonF* poly
                                              , const QPointF& pos
                                              ) const
        {
          if (direction() == connectable::direction::IN)
          {
            cap::add_incoming (poly, pos);
          }
          else
          {
            cap::add_outgoing (poly, pos);
          }
        }

        void top_level_port_item::add_cap_for_direction ( QPolygonF* poly
                                                        , const QPointF& pos
                                                        ) const
        {
          if (direction() == connectable::direction::IN)
          {
            cap::add_outgoing (poly, pos);
          }
          else
          {
            cap::add_incoming (poly, pos);
          }
        }

        QPainterPath port_item::shape () const
        {
          const qreal lengthHalf (length() / 2.0); // hardcoded constant

          QPolygonF poly;
          const qreal y (size::port::height() / 2.0);
          poly << QPointF ( lengthHalf - size::cap::length(),  y)
               << QPointF (-lengthHalf                      ,  y)
               << QPointF (-lengthHalf                      , -y)
               << QPointF ( lengthHalf - size::cap::length(), -y)
            ;

          add_cap_for_direction
            (&poly, QPointF (lengthHalf - size::cap::length(), 0.0));

          poly = QTransform().rotate (angle (orientation())).map (poly);

          QPainterPath path;
          path.addPolygon (poly);
          path.closeSubpath();

          return path;
        }

        QRectF port_item::bounding_rect (bool cap, int cap_factor) const
        {
          const qreal addition ( cap
                               ? 0.0
                               : size::cap::length() * cap_factor
                               );
          const qreal lengthHalf ((length() - addition) / 2.0);                                  // hardcoded constant

          switch (orientation())
          {
          case port::orientation::NORTH:
            return QRectF ( -(size::port::height() / 2.0)
                          , -lengthHalf + addition
                          , size::port::height()
                          , length() - addition
                          );

          case port::orientation::SOUTH:
            return QRectF ( -(size::port::height() / 2.0)
                          , -lengthHalf
                          , size::port::height()
                          , length() - addition
                          );

          case port::orientation::EAST:
            return QRectF ( -lengthHalf
                          , -(size::port::height() / 2.0)
                          , length() - addition
                          , size::port::height()
                          );

          case port::orientation::WEST:
            return QRectF ( -lengthHalf + addition
                          , -(size::port::height() / 2.0)
                          , length() - addition
                          , size::port::height()
                          );
          default:
            throw std::runtime_error("invalid port direction!");
          }
        }

        void port_item::paint ( QPainter *painter
                              , const QStyleOptionGraphicsItem *
                              , QWidget *
                              )
        {
          style::draw_shape (this, painter);

          painter->setPen (QPen ( QBrush (style<QColor> ("text_color"))
                                , style<qreal> ("text_line_thickness")
                                )
                          );
          painter->setBackgroundMode (Qt::TransparentMode);

          const QRectF area (bounding_rect (false, 1));

          if ( orientation() == port::orientation::NORTH
             || orientation() == port::orientation::SOUTH
             )
          {
            const qreal degrees (90.0);                                                 // hardcoded constant

            painter->save();
            painter->rotate (degrees);
            painter->drawText ( QTransform().rotate(-degrees).mapRect (area)
                              , Qt::AlignCenter
                              , QString::fromStdString (name())
                              );
            painter->restore();
          }
          else
          {
            painter->drawText(area, Qt::AlignCenter, QString::fromStdString (name()));
          }
        }

        void port_item::type_or_name_changed
          ( const QObject* origin
          , const data::handle::port& changed_handle
          , const QString&
          )
        {
          if (changed_handle == handle())
          {
            refresh_content();
            update();
          }
        }

        void port_item::property_changed
          ( const QObject* origin
          , const data::handle::port& changed_handle
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
