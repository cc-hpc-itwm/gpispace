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
        namespace connection
        {
          item::item (bool reading_only)
            : graph::item()
            , _start (NULL)
            , _end (NULL)
            , _fixed_points()
            , _reading_only (reading_only)
          {
            setZValue (-1);                                                          // hardcoded constant
          }
          item::~item()
          {
            start (NULL);
            end (NULL);
          }

          connectable_item* item::start() const
          {
            return _start;
          }
          connectable_item* item::start (connectable_item* start_)
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
          connectable_item* item::end() const
          {
            return _end;
          }
          connectable_item* item::end (connectable_item* end_)
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

          connectable_item* item::non_free_side() const
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
          connectable_item* item::free_side(connectable_item* item)
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

          const QList<QPointF>& item::fixed_points() const
          {
            return _fixed_points;
          }

          const bool& item::reading_only() const
          {
            return _reading_only;
          }
          const bool& item::reading_only (const bool& reading_only_)
          {
            return _reading_only = reading_only_;
          }
        }
      }
    }
  }
}
