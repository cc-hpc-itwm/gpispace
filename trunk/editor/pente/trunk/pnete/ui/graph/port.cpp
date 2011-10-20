// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/port.hpp>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QPainter>
#include <QStaticText>
#include <QDebug>
#include <QMenu>

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
        namespace port
        {
          namespace detail
          {
            static void set_orientation ( ::we::type::property::type* prop
                                        , const orientation::type& o
                                        )
            {
              static util::property::setter s ("orientation");
              s.set (prop, orientation::show (o));
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
                if (qgraphicsitem_cast<const item*>(gi)->we_type() == type)
                  {
                    return boost::optional<const QColor&> (color);
                  }
              }

            return boost::none;
          }

          item::item
          ( port_type& port
          , connectable::direction::type direction
          , boost::optional< ::xml::parse::type::type_map_type&> type_map
          , transition::item* parent
          )
            : connectable::item (direction, type_map, parent, &port.prop)
            , _port (port)
            , _orientation ()
            , _dragging (false)
            , _drag_start (0.0, 0.0)
            , _length (size::port::width())
            , _menu_context()
          {
            access_style().push<qreal>
              ( "border_thickness"
              , style::mode::NORMAL
              , boost::bind (&thicker_if_name, "in", _1)
              );

            static const QColor background_color_long (Qt::darkBlue);
            static const QColor background_color_string (Qt::yellow);
            static const QColor background_color_control (Qt::red);
            static const QColor backgr (Qt::green);

            access_style().push<QColor>
              ( "background_color"
              , style::mode::NORMAL
              , boost::bind (&color_if_type, "long", background_color_long, _1)
              );
            access_style().push<QColor>
              ( "background_color"
              , style::mode::HIGHLIGHT
              , boost::bind (&color_if_type, "long", backgr, _1)
              );
            access_style().push<QColor>
              ( "background_color"
              , style::mode::NORMAL
              , boost::bind (&color_if_type, "string", background_color_string, _1)
              );
//             access_style().push<QColor>
//               ( "background_color"
//               , style::mode::NORMAL
//               , boost::bind (&color_if_type, "control", background_color_control, _1)
//               );

            static const QColor text_color_long (Qt::white);
            static const QColor text_color_string (Qt::gray);

            access_style().push<QColor>
              ( "text_color"
              , style::mode::NORMAL
              , boost::bind (&color_if_type, "long", text_color_long, _1)
              );
            access_style().push<QColor>
              ( "text_color"
              , style::mode::NORMAL
              , boost::bind (&color_if_type, "string", text_color_string, _1)
              );

            set_just_orientation_but_not_in_property
              ( direction == connectable::direction::OUT
              ? orientation::EAST
              : orientation::WEST
              );

            //            setAcceptHoverEvents (true);
            //! \todo verbose name

            refresh_tooltip();

            _length = qMax( _length
                          , QStaticText(QString::fromStdString (name())).size().width()
                          + 2 * size::cap::length()
                          );

            init_menu_context();
          }

          void item::init_menu_context()
          {
            QAction* action_set_type (_menu_context.addAction(tr("Set type")));
            connect (action_set_type, SIGNAL(triggered()), SLOT(slot_set_type()));

            _menu_context.addSeparator();

            QAction* action_delete (_menu_context.addAction(tr("Delete")));
            connect (action_delete, SIGNAL(triggered()), SLOT(slot_delete()));
          }

          void item::slot_set_type ()
          {
            qDebug() << "port::slot_set_type()";
          }

          void item::slot_delete ()
          {
            //delete_connection();
            //scene()->removeItem (this);
          }

          void item::contextMenuEvent (QGraphicsSceneContextMenuEvent* event)
          {
            graph::item::contextMenuEvent (event);

            if (!event->isAccepted())
              {
                _menu_context.popup (event->screenPos());
                event->accept();
              }
          }

          void item::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
          {
            if (!_dragging)
              {
                connectable::item::mouseReleaseEvent (event);
                return;
              }

            _dragging = false;
            event->accept();
          }

          void item::mousePressEvent (QGraphicsSceneMouseEvent* event)
          {
            if (event->buttons() & Qt::RightButton)
              {
                event->ignore();
                return;
              }

            if (event->modifiers() == Qt::ControlModifier)
              {
                _dragging = true;
                _drag_start = event->pos();
                event->accept();
                return;
              }

            //! \note Only allow one connection on ports.
            if (!_connections.isEmpty())
              {
                event->ignore();
                return;
              }

            connectable::item::mousePressEvent (event);
          }

          bool item::is_connectable_with (const connectable::item* item) const
          {
            //! \note Only allow one connection on ports.
            return _connections.isEmpty()
              && connectable::item::is_connectable_with (item);
          }

          QPointF item::fitting_position (QPointF position)
          {
            if (!parentItem())
              {
                return style::raster::snap (position);
              }

            const QPointF minimum_distance ( boundingRect().width() / 2.0 + 1.0    // hardcoded constants
                                           , boundingRect().height() / 2.0 + 1.0   // hardcoded constants
                                           );

            const QRectF bounding (parentItem()->boundingRect());

            qreal to_left (position.x() - bounding.left());
            qreal to_right (position.x() - bounding.right());
            qreal to_top (position.y() - bounding.top());
            qreal to_bottom (position.y() - bounding.bottom());
            to_left *= to_left;
            to_right *= to_right;
            to_top *= to_top;
            to_bottom *= to_bottom;

            orientation ( qMin (to_top, to_bottom) < qMin (to_left, to_right)
                        ? ( to_top < to_bottom
                          ? orientation::NORTH
                          : orientation::SOUTH
                          )
                        : ( to_left < to_right
                          ? orientation::WEST
                          : orientation::EAST
                          )
                        );

            if(  orientation() == orientation::WEST
              || orientation() == orientation::EAST
              )
              {
                position.setX ( orientation() == orientation::WEST
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
                position.setY ( orientation() == orientation::NORTH
                              ? bounding.top()
                              : bounding.bottom()
                              );
              }

            return style::raster::snap (position);
          }

          void item::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
          {
            if(!_dragging)
              {
                connectable::item::mouseMoveEvent (event);
                return;
              }

            const QPointF old_location (pos());
            const orientation::type old_orientation (orientation());
            const QPointF new_location (pos() + event->pos() - _drag_start);

            setPos (fitting_position (new_location));

            // do not move, when now colliding with a different port
            foreach (QGraphicsItem* collidingItem, collidingItems())
              {
                if ( qgraphicsitem_cast<item*>(collidingItem)
                   && collidingItem->parentItem() == parentItem()
                   )
                  {
                    orientation (old_orientation);
                    setPos (old_location);
                    event->ignore();
                    return;
                  }
              }
            event->accept();
            scene()->update (boundingRect().translated (old_location));
            scene()->update (boundingRect().translated (pos()));
          }

          const qreal& item::length() const
          {
            return _length;
          }

          const std::string& item::name() const
          {
            return port().name;
          }
          const std::string& item::we_type() const
          {
            return connectable::item::we_type (port().type);
          }

          const orientation::type&
          item::orientation() const
          {
            return _orientation;
          }
          const orientation::type&
          item::orientation (const orientation::type& orientation_)
          {
            _orientation = orientation_;

            detail::set_orientation (&_port.prop, orientation());

            return orientation();
          }

          void item::set_just_orientation_but_not_in_property
          (const orientation::type& orientation_)
          {
            _orientation = orientation_;
          }

          // void item::delete_connection()
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

          void item::refresh_tooltip()
          {
            setToolTip (QString::fromStdString (name()) + " :: " + QString::fromStdString (we_type()));
          }

          static qreal angle (const orientation::type& o)
          {
            switch (o)
              {
              case orientation::NORTH: return -90.0;
              case orientation::EAST: return 0.0;
              case orientation::SOUTH: return 90.0;
              case orientation::WEST: return 180.0;
              default:
                throw std::runtime_error ("angle (NON_ORIENTATION)!?");
              }
          }

          QPainterPath item::shape () const
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

          QRectF item::bounding_rect (bool cap, int cap_factor) const
          {
            const qreal addition ( cap
                                 ? 0.0
                                 : size::cap::length() * cap_factor
                                 );
            const qreal lengthHalf ((length() - addition) / 2.0);                                  // hardcoded constant

            switch (orientation())
              {
              case orientation::NORTH:
                return QRectF ( -(size::port::height() / 2.0)
                              , -lengthHalf + addition
                              , size::port::height()
                              , length() - addition
                              );

              case orientation::SOUTH:
                return QRectF ( -(size::port::height() / 2.0)
                              , -lengthHalf
                              , size::port::height()
                              , length() - addition
                              );

              case orientation::EAST:
                return QRectF ( -lengthHalf
                              , -(size::port::height() / 2.0)
                              , length() - addition
                              , size::port::height()
                              );

              case orientation::WEST:
                return QRectF ( -lengthHalf + addition
                              , -(size::port::height() / 2.0)
                              , length() - addition
                              , size::port::height()
                              );
              default:
                throw std::runtime_error("invalid port direction!");
              }
          }

          QRectF item::boundingRect () const
          {
            return bounding_rect ();
          }

          QColor queryColorForType (const QString& type)
          {
            //! \note Colors shamelessly stolen from PSPro.
            //! \todo Maybe also do a gradient? Maybe looks awesome.
            if (type.startsWith ("seismic"))
              {
                return QColor (0, 130, 250);                                           // hardcoded constant
              }
            else if (type.startsWith ("velocity"))
              {
                return QColor (248, 248, 6);                                           // hardcoded constant
              }
            else
              {
                return QColor (255, 255, 255);                                         // hardcoded constant
              }
          }

          void item::paint ( QPainter *painter
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

            if ( orientation() == orientation::NORTH
               || orientation() == orientation::SOUTH
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
}
