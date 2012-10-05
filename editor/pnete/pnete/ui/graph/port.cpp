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

#include <util/property.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace detail
        {
          static void set_orientation ( ::we::type::property::type* prop
                                      , const port::orientation::type& o
                                      )
          {
            static util::property::setter s ("orientation");
            s.set (prop, port::orientation::show (o));
          }
        }

        static boost::optional<const qreal&>
        thicker_if_name (const std::string type, const graph::item* gi)
        {
          static const qreal v (4.0);

          return style::predicate::generic_if<qreal>
            ( style::predicate::_and
              ( style::predicate::predicate (&style::predicate::is_port)
              , style::predicate::on<std::string>
                ( &style::predicate::port::name
                , boost::bind (&style::predicate::equals, type, _1)
                )
              )
            , v
            , gi
            );
        }

        static boost::optional<const QColor&>
        color_if_type ( const std::string& type
                      , const QColor& color
                      , const graph::item* gi
                      )
        {
          if (style::predicate::is_port (gi))
          {
            if (qgraphicsitem_cast<const port_item*>(gi)->we_type() == type)
            {
              return boost::optional<const QColor&> (color);
            }
          }

          return boost::none;
        }

        port_item::port_item
        ( port_type& port
        , connectable::direction::type direction
        , boost::optional< ::xml::parse::type::type_map_type&> type_map
        , transition_item* parent
        )
          : connectable_item (direction, type_map, parent, &port.prop)
          , _port (port)
          , _orientation ()
          , _length (size::port::width())
        {
          access_style().push<qreal>
            ( "border_thickness"
            , mode::NORMAL
            , boost::bind (&thicker_if_name, "in", _1)
            );

          static const QColor background_color_long (Qt::darkBlue);
          static const QColor background_color_string (Qt::yellow);
          static const QColor background_color_control (Qt::red);
          static const QColor backgr (Qt::green);

          access_style().push<QColor>
            ( "background_color"
            , mode::NORMAL
            , boost::bind (&color_if_type, "long", background_color_long, _1)
            );
          access_style().push<QColor>
            ( "background_color"
            , mode::HIGHLIGHT
            , boost::bind (&color_if_type, "long", backgr, _1)
            );
          access_style().push<QColor>
            ( "background_color"
            , mode::NORMAL
            , boost::bind (&color_if_type, "string", background_color_string, _1)
            );
//             access_style().push<QColor>
//               ( "background_color"
//               , mode::NORMAL
//               , boost::bind (&color_if_type, "control", background_color_control, _1)
//               );

          static const QColor text_color_long (Qt::white);
          static const QColor text_color_string (Qt::gray);

          access_style().push<QColor>
            ( "text_color"
            , mode::NORMAL
            , boost::bind (&color_if_type, "long", text_color_long, _1)
            );
          access_style().push<QColor>
            ( "text_color"
            , mode::NORMAL
            , boost::bind (&color_if_type, "string", text_color_string, _1)
            );

          set_just_orientation_but_not_in_property
            ( direction == connectable::direction::OUT
            ? port::orientation::EAST
            : port::orientation::WEST
            );

          //            setAcceptHoverEvents (true);
          //! \todo verbose name

          refresh_tooltip();

          _length = qMax( _length
                        , QStaticText(QString::fromStdString (name())).size().width()
                        + 2 * size::cap::length()
                        );
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
          const port::orientation::type old_orientation (orientation());

          connectable_item::setPos (fitting_position (new_position));

          // do not move, when now colliding with a different port
          foreach (QGraphicsItem* collidingItem, collidingItems())
          {
            if ( qgraphicsitem_cast<port_item*>(collidingItem)
               && collidingItem->parentItem() == parentItem()
               )
            {
              orientation (old_orientation);
              connectable_item::setPos (old_position);

              return;
            }

            if ( transition_item* transition
               = qgraphicsitem_cast<transition_item*> (collidingItem)
               )
            {
              if (parentItem() != transition)
              {
                orientation (old_orientation);
                connectable_item::setPos (old_position);

                return;
              }
            }
          }
        }

        bool port_item::is_connectable_with (const connectable_item* item) const
        {
          //! \note Only allow one connection on ports.
          return _connections.isEmpty()
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

            orientation ( qMin (to_top, to_bottom) < qMin (to_left, to_right)
                        ? ( to_top < to_bottom
                          ? port::orientation::NORTH
                          : port::orientation::SOUTH
                          )
                        : ( to_left < to_right
                          ? port::orientation::WEST
                          : port::orientation::EAST
                          )
                        );

            if(  orientation() == port::orientation::WEST
              || orientation() == port::orientation::EAST
              )
            {
              position.setX ( orientation() == port::orientation::WEST
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
              position.setY ( orientation() == port::orientation::NORTH
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
          return port().name;
        }
        const std::string& port_item::we_type() const
        {
          return connectable_item::we_type (port().type);
        }

        const port::orientation::type&
        port_item::orientation() const
        {
          return _orientation;
        }
        const port::orientation::type&
        port_item::orientation (const port::orientation::type& orientation_)
        {
          _orientation = orientation_;

          detail::set_orientation (&_port.prop, orientation());

          return orientation();
        }

        void port_item::set_just_orientation_but_not_in_property
        (const port::orientation::type& orientation_)
        {
          _orientation = orientation_;
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

        void port_item::refresh_tooltip()
        {
          setToolTip (QString::fromStdString (name()) + " :: " + QString::fromStdString (we_type()));
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

          if (direction() == connectable::direction::IN)
          {
            cap::add_incoming (&poly, QPointF (lengthHalf - size::cap::length(), 0.0));   // hardcoded constant
          }
          else
          {
            cap::add_outgoing (&poly, QPointF (lengthHalf - size::cap::length(), 0.0));
          }

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
      }
    }
  }
}

