// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <pnete/ui/execution_monitor_worker_model.hpp>

#include <fhg/assert.hpp>

#include <util-generic/this_bound_mem_fn.hpp>

#include <QTimer>

#include <algorithm>
#include <string>
#include <utility>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      worker_model::value_type::value_type
        ( timestamp_type t
        , boost::optional<duration_type> d
        , QString id, QString name, state_type state_
        )
          : _timestamp (t)
          , _duration (d)
          , _id (id)
          , _name (name)
          , _state (sdpa::daemon::NotificationEvent::STATE_STARTED)
      {
        state (state_, t);
      }

      worker_model::timestamp_type worker_model::value_type::timestamp() const
      {
        return _timestamp;
      }

      void worker_model::value_type::duration (boost::optional<duration_type> d)
      {
        _duration = d;
      }
      boost::optional<worker_model::duration_type>
        worker_model::value_type::duration() const
      {
        return _duration;
      }

      QString worker_model::value_type::id() const
      {
        return _id;
      }

      worker_model::state_type worker_model::value_type::state() const
      {
        return _state;
      }
      void worker_model::value_type::state (state_type state_, timestamp_type now)
      {
        switch (state_)
        {
        case sdpa::daemon::NotificationEvent::STATE_CANCELED:
        case sdpa::daemon::NotificationEvent::STATE_FAILED:
        case sdpa::daemon::NotificationEvent::STATE_FINISHED:
          duration (now - timestamp());
          break;

        case sdpa::daemon::NotificationEvent::STATE_STARTED:
          break;
        }

        _state = state_;
      }

      worker_model::worker_model (QObject* parent)
        : QAbstractItemModel (parent)
        , _workers()
        , _worker_containers()
        , _base_time (QDateTime::currentDateTime())
        , _event_queue()
        , _queued_events()
      {
        QTimer* timer (new QTimer (this));
        connect (timer, SIGNAL (timeout()), SLOT (handle_events()));
        timer->start (100);
      }

      void worker_model::append_event (logging::message const& event)
      {
        if (event._category != sdpa::daemon::gantt_log_category)
        {
          return;
        }

        std::lock_guard<std::mutex> guard (_event_queue);
        _queued_events.push_back (std::move (event));
      }

      namespace
      {
        template<typename Timepoint>
          QDateTime to_qt (Timepoint timepoint)
        {
          using namespace std::chrono;
          return QDateTime::fromMSecsSinceEpoch
            ( duration_cast<duration<qint64, milliseconds::period>>
                (timepoint.time_since_epoch())
              .count()
            );
        }
      }

      void worker_model::handle_events()
      {
        boost::optional<QModelIndex> ul, br;

        QVector<logging::message> events;
        {
          std::lock_guard<std::mutex> guard (_event_queue);
          std::swap (events, _queued_events);
        }

        for (logging::message log_event : events)
        {
          const sdpa::daemon::NotificationEvent event (log_event._content);

          const long time
            ( to_qt (log_event._timestamp).toMSecsSinceEpoch()
            - _base_time.toMSecsSinceEpoch()
            );

          const QString activity_id
            (QString::fromStdString (event.activity_id()));

          for (std::string const& worker_std : event.components())
          {
            const QString worker (QString::fromStdString (worker_std));

            const bool is_new_worker (!_workers.contains (worker));
            const int row
              (is_new_worker ? _workers.size() : _workers.indexOf (worker));
            if (is_new_worker)
            {
              beginInsertRows (QModelIndex(), row, row);
              _workers << worker;
              endInsertRows();
            }

            std::vector<value_type>& container (_worker_containers[worker]);

            //! \note We assume that there are no message reorderings,
            //! thus the message always is for the current or a new
            //! activity. This may be wrong due to UDP, but hopefully
            //! is not.  Receiving back-dated messages currently leads
            //! to inserting a new activity where timestamp <
            //! current.timestamp, which will throw or result in other
            //! weird behaviour.
            if (container.empty() || container.back().id() != activity_id)
            {
              fhg_assert ( std::find_if
                           ( container.begin()
                           , container.end()
                           , [&activity_id] (value_type const& v)
                           {
                             return v.id() == activity_id;
                           }
                           ) == container.end()
                         , "no previous interval shall have the same id as a new one"
                         );

              if (!container.empty())
              {
                value_type& current (container.back());

                fhg_assert ( current.timestamp() <= time
                           , "only appending: current is after inserted"
                           );

                fhg_assert ( !current.duration()
                           || current.timestamp() + *current.duration() <= time
                           , "only appending: interval of current intersects new"
                           );

                if (!current.duration())
                {
                  current.state (sdpa::daemon::NotificationEvent::STATE_FAILED, time);
                }
              }

              container.push_back
                ( value_type ( time
                             , boost::none
                             , activity_id
                             , QString::fromStdString (event.activity_name())
                             , event.activity_state()
                             )
                );
            }
            else
            {
              container.back().state (event.activity_state(), time);
            }

            (ul ? br : ul) = index (row, 0);
          }
        }

        if (ul)
        {
          emit dataChanged (*ul, br ? *br : *ul);
        }
      }

      int worker_model::rowCount (QModelIndex const& parent) const
      {
        return parent.isValid() ? 0 : _workers.size();
      }
      int worker_model::columnCount (QModelIndex const& parent) const
      {
        return parent.isValid() ? 0 : 1;
      }

      namespace
      {
        bool is_before ( worker_model::timestamp_type t
                       , worker_model::value_type val
                       )
        {
          return !val.duration() || t < (val.timestamp() + *val.duration());
        }

        worker_model::subrange_type find_subrange
          ( std::vector<worker_model::value_type>::const_iterator begin
          , std::vector<worker_model::value_type>::const_iterator end
          , worker_model::timestamp_type from
          , worker_model::timestamp_type to
          )
        {
          const std::vector<worker_model::value_type>::const_iterator lower
            (std::upper_bound (begin, end, from, is_before));

          return worker_model::subrange_type
            ( lower
            , std::lower_bound
              ( lower, end
              , to
              , [] ( worker_model::value_type const& v
                   , worker_model::timestamp_type const& t
                   )
              {
                return v.timestamp() < t;
              }
              )
            );
        }
      }

      QVariant worker_model::data (QModelIndex const& index, int role) const
      {
        if (index.isValid())
        {
          if (role == name_role || role == Qt::DisplayRole)
          {
            return _workers[index.row()];
          }
          else if (role == current_interval_role)
          {
            return QVariant::fromValue<boost::optional<value_type>>
              (_worker_containers[_workers[index.row()]].back());
          }
          else if (role == range_getter_role)
          {
            return QVariant::fromValue<subrange_getter_type>
              ( std::bind
                ( &find_subrange
                , _worker_containers.find (_workers[index.row()])->begin()
                , _worker_containers.find (_workers[index.row()])->end()
                , std::placeholders::_1
                , std::placeholders::_2
                )
              );
          }
        }
        return QVariant();
      }

      QVariant worker_model::headerData (int, Qt::Orientation, int role) const
      {
        if (role == base_time_role)
        {
          return _base_time;
        }
        return QVariant();
      }

      QModelIndex worker_model::index
        (int row, int column, QModelIndex const& parent) const
      {
        return !hasIndex (row, column, parent)
          ? QModelIndex()
          : createIndex (row, column, nullptr);
      }

      QModelIndex worker_model::parent (QModelIndex const&) const
      {
        return QModelIndex();
      }

      void worker_model::clear()
      {
        beginResetModel();

        _workers.clear();
        _worker_containers.clear();
        _base_time = QDateTime::currentDateTime();

        endResetModel();
      }
      bool worker_model::removeRows
        (int row, int count, QModelIndex const& parent)
      {
        beginRemoveRows (parent, row, row + count - 1);
        while (count --> 0)
        {
          _worker_containers.remove (_workers[row]);
          _workers.removeAt (row);
        }
        if (_workers.empty())
        {
          _base_time = QDateTime::currentDateTime();
        }
        endRemoveRows();
        return true;
      }
    }
  }
}
