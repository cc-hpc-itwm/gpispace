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

#include <pnete/ui/graph/style/cap.hpp>
#include <pnete/ui/graph/style/isc13.hpp>
#include <pnete/ui/graph/style/predicate.hpp>
#include <pnete/ui/graph/style/raster.hpp>
#include <pnete/ui/graph/style/size.hpp>

#include <we/type/value/path/split.hpp>

#include <fhg/util/num.hpp>
#include <fhg/util/parse/require.hpp>

#include <util/qt/cast.hpp>

#include <xml/parse/type/port.hpp>

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
          boost::optional<QBrush> color_if_type
            (const base_item* item, const QBrush& c, const QString& type)
          {
            return boost::make_optional
              ( fhg::util::qt::throwing_qobject_cast<const port_item*>
                (item)->handle().type() == type
              , c
              );
          }
        }

        port_item::port_item ( const data::handle::port& handle
                             , transition_item* parent
                             )
          : connectable_item (parent)
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

          handle.connect_to_change_mgr
            (this, "port_deleted", "data::handle::port");

          //            setAcceptHoverEvents (true);
          //! \todo verbose name

          refresh_content();

          style::isc13::add_colors_for_types (&_style, color_if_type);
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
          const bool outer (parentItem() != NULL);
          connectable_item::setPos (new_position, outer);
        }

        void port_item::setPos (const QPointF& new_position)
        {
          const QPointF old_position (pos());

          setPos_no_collision_detection (fitting_position (new_position));

          // do not move, when now colliding with a different port
          foreach (QGraphicsItem* collidingItem, collidingItems())
          {
            if ( qgraphicsitem_cast<port_item*>(collidingItem)
               && collidingItem->parentItem() == parentItem()
               )
            {
              setPos_no_collision_detection (old_position);

              return;
            }

            if ( transition_item* transition
               = qgraphicsitem_cast<transition_item*> (collidingItem)
               )
            {
              if (parentItem() != transition)
              {
                setPos_no_collision_detection (old_position);

                return;
              }
            }
          }
        }

        bool port_item::is_connectable_with (const connectable_item* item) const
        {
          const port_item* rhs (qobject_cast<const port_item*> (item));
          return may_be_connected()
            && (!rhs || handle().is_connectable (rhs->handle()))
            && connectable_item::is_connectable_with (item);
        }

        bool port_item::may_be_connected() const
        {
          //! \note Only allow one connection.
          return _associations.isEmpty();
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
            return handle().is_input()
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
          if (handle().is_input())
          {
            cap::add_incoming (poly, pos);
          }
          else if (handle().is_output())
          {
            cap::add_outgoing (poly, pos);
          }
          else
          {
            const boost::optional<std::string> tunnel_direction
              (handle().get().properties().get
                ("fhg.pnete.tunnel.direction")
              );

            if (tunnel_direction)
            {
              if (*tunnel_direction == "out")
              {
                cap::add_outgoing (poly, pos);
              }
              else if (*tunnel_direction == "in")
              {
                cap::add_incoming (poly, pos);
              }
              else
              {
                throw std::runtime_error
                  ("bad fhg.pnete.tunnel.direction (neither 'in' nor 'out')");
              }
            }
            else
            {
              //! \todo Cap for tunnel ports.
            }
          }
        }

        void top_level_port_item::add_cap_for_direction ( QPolygonF* poly
                                                        , const QPointF& pos
                                                        ) const
        {
          if (handle().is_input())
          {
            cap::add_outgoing (poly, pos);
          }
          else if (handle().is_output())
          {
            cap::add_incoming (poly, pos);
          }
          else
          {
            throw std::runtime_error ( "requested cap for non-input, non-output "
                                       "port (i.e. tunnel), while tunnels should"
                                       " not be drawn on top level!"
                                     );
          }
        }

        QPainterPath port_item::shape () const
        {
          const qreal lengthHalf (length() / 2.0); // hardcoded constant

          QPolygonF poly;
          const qreal y (size::port::height() / 2.0);
          poly << QPointF ( lengthHalf, y)
               << QPointF (-lengthHalf, y)
               << QPointF (-lengthHalf, -y)
               << QPointF ( lengthHalf, -y);

          add_cap_for_direction
            (&poly, QPointF (lengthHalf, 0.0));

          poly = QTransform().rotate (angle (orientation())).map (poly);

          QPainterPath path;
          path.addPolygon (poly);
          path.closeSubpath();

          return path;
        }

        namespace
        {
          QRectF no_cap_rect
            (const qreal& length, const port::orientation::type& orientation)
          {
            const qreal lengthHalf (length / 2.0); // hardcoded constant

            QPolygonF poly;
            const qreal y (size::port::height() / 2.0);
            poly << QPointF ( lengthHalf, y)
                 << QPointF (-lengthHalf, y)
                 << QPointF (-lengthHalf, -y)
                 << QPointF ( lengthHalf, -y);

            poly = QTransform().rotate (angle (orientation)).map (poly);

            QPainterPath path;
            path.addPolygon (poly);
            path.closeSubpath();

            return path.boundingRect();
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

          const QRectF area (no_cap_rect (length(), orientation()));

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

        void port_item::port_deleted
          (const data::handle::port& changed)
        {
          if (changed == handle())
          {
            scene()->removeItem (this);
            deleteLater();
          }
        }

        void port_item::type_or_name_changed
          ( const data::handle::port& changed_handle
          , const QString&
          )
        {
          if (changed_handle == handle())
          {
            refresh_content();
            update();
          }
        }

        namespace
        {
          qreal read_qreal (const std::string& inp)
          {
            util::parse::position_string pos (inp);
            fhg::util::parse::require::skip_spaces (pos);
            return util::read_double (pos);
          }
        }

        void port_item::property_changed
          ( const data::handle::port& changed_handle
          , const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& value
          )
        {
          if (changed_handle == handle())
          {
            const std::string required_position_variable
              ( parentItem() == NULL
              ? "fhg.pnete.position"
              : "fhg.pnete.outer_position"
              );

            const ::we::type::property::path_type path
              (pnet::type::value::path::split (key));


            ::we::type::property::path_type::const_iterator pos (path.begin());
            ::we::type::property::path_type::const_iterator const end (path.end());

            if (  std::distance (pos, end) == 4
               && *pos == "fhg" && *boost::next (pos) == "pnete"
               && (  *boost::next (boost::next (pos)) == "position"
                  || *boost::next (boost::next (pos)) == "outer_position"
                  )
               )
            {
              ++pos;
              ++pos;

              if (  (parentItem() == NULL && *pos == "position")
                 || (parentItem() != NULL && *pos == "outer_position")
                 )
              {
                ++pos;

                if (*pos == "x")
                {
                  set_just_pos_but_not_in_property
                    (QPointF (read_qreal (value), this->pos().y()));
                }
                else if (*pos == "y")
                {
                  set_just_pos_but_not_in_property
                    (QPointF (this->pos().x(), read_qreal (value)));
                }
              }
            }
            else
            {
              handle_property_change (key, value);
            }
          }
        }
      }
    }
  }
}
