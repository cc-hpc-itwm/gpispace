// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/connection.hpp>

#include <QPainter>

#include <pnete/ui/graph/connectable_item.hpp>
#include <pnete/ui/graph/style.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        connection::connection (bool reading_only)
          : item()
          , _start (NULL)
          , _end (NULL)
          , _fixed_points()
          , _reading_only (reading_only)
        {
          setZValue (-1);                                                          // hardcoded constant
        }
        connection::~connection()
        {
          start (NULL);
          end (NULL);
        }

        connectable_item* connection::start() const
        {
          return _start;
        }
        connectable_item* connection::start (connectable_item* start_)
        {
          if (_start)
          {
            _start->remove_connection (this);
          }
          _start = start_;
          if (_start)
          {
            _start->add_connection (this);
          }
          return _start;
        }
        connectable_item* connection::end() const
        {
          return _end;
        }
        connectable_item* connection::end (connectable_item* end_)
        {
          if (_end)
          {
            _end->remove_connection (this);
          }
          _end = end_;
          if (_end)
          {
            _end->add_connection (this);
          }
          return _end;
        }

        connectable_item* connection::non_free_side() const
        {
          if (_start && _end)
          {
            throw std::runtime_error ( "can't get a non_free side as both "
                                       "sides are connected."
                                     );
          }
          if (!_start && !_end)
          {
            throw std::runtime_error ( "can't get a non_free side as both "
                                       "sides are not connected."
                                     );
          }
          return _start ? _start : _end;
        }
        connectable_item* connection::free_side(connectable_item* item)
        {
          if (_start && _end)
          {
            throw std::runtime_error ( "can't connect free side, as there "
                                       "is none."
                                     );
          }
          if (!_start && !_end)
          {
            throw std::runtime_error ("can't connect free side, as both are.");
          }
          return _end ? start (item) : end (item);
        }

        const QList<QPointF>& connection::fixed_points() const
        {
          return _fixed_points;
        }

        const bool& connection::reading_only() const
        {
          return _reading_only;
        }
        const bool& connection::reading_only (const bool& reading_only_)
        {
          return _reading_only = reading_only_;
        }

        QPainterPath connection::shape() const
        {
          return style::connectionShape (this);
        }

        QRectF connection::boundingRect() const
        {
          return style::connectionBoundingRect (this);
        }

        void connection::paint( QPainter* painter
                              , const QStyleOptionGraphicsItem*
                              , QWidget*
                              )
        {
          style::connectionPaint (painter, this);
        }
      }
    }
  }
}
