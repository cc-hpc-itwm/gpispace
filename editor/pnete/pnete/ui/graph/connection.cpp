// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connection.hpp>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/scene.hpp>
#include <pnete/ui/graph/style/association.hpp>

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
        connection_item::connection_item
          ( connectable_item* start
          , connectable_item* end
          , const boost::optional<data::handle::connect>& handle
          , bool read
          )
            : base_item()
            , _handle (handle)
            , _start (start)
            , _end (end)
            , _fixed_points()
            , _read (read)
        {
          start->add_connection (this);
          end->add_connection (this);

          setZValue (-1);                                                          // hardcoded constant
          set_just_pos_but_not_in_property (0.0, 0.0);
        }
        connection_item::~connection_item()
        {
          _start->remove_connection (this);
          _end->remove_connection (this);
        }

        connectable_item* connection_item::start() const
        {
          return _start;
        }
        connectable_item* connection_item::end() const
        {
          return _end;
        }

        const QList<QPointF>& connection_item::fixed_points() const
        {
          return _fixed_points;
        }

        const QList<QPointF>&
        connection_item::fixed_points (const QList<QPointF>& fixed_points_)
        {
          return _fixed_points = fixed_points_;
        }

        const bool& connection_item::read() const
        {
          return _read;
        }
        const bool& connection_item::read (const bool& read_)
        {
          return _read = read_;
        }

        QPainterPath connection_item::shape () const
        {
          QList<QPointF> allPoints;
          allPoints.push_back (start()->scenePos());
          foreach (QPointF point, fixed_points())
          {
            allPoints.push_back (point);
          }
          allPoints.push_back (end()->scenePos());

          return association::shape (allPoints);
        }

        void connection_item::paint ( QPainter* painter
                                    , const QStyleOptionGraphicsItem*
                                    , QWidget*
                                    )
        {
          style::draw_shape (this, painter);
        }

        void connection_item::mousePressEvent (QGraphicsSceneMouseEvent* event)
        {
          //! \todo Add ability to set control points.
          if (event->modifiers() == Qt::ControlModifier)
          {
            event->ignore();
          }
          else
          {
            base_item::mousePressEvent (event);
          }
        }
      }
    }
  }
}
