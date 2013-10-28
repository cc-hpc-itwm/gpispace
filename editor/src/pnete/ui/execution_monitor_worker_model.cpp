// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/execution_monitor_worker_model.hpp>

#include <fhg/assert.hpp>

#include <QTimer>

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
          , _state (sdpa::daemon::NotificationEvent::STATE_CREATED)
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
        case sdpa::daemon::NotificationEvent::STATE_CANCELLED:
        case sdpa::daemon::NotificationEvent::STATE_FAILED:
        case sdpa::daemon::NotificationEvent::STATE_FINISHED:
          duration (now - timestamp());

        case sdpa::daemon::NotificationEvent::STATE_CREATED:
        case sdpa::daemon::NotificationEvent::STATE_STARTED:
          ;
        }

        _state = state_;
      }

      namespace
      {
        struct delegating_fhglog_appender : public log::Appender
        {
          template<typename Append, typename Flush>
            static ptr_t create (Append append, Flush flush)
          {
            return ptr_t (new delegating_fhglog_appender (append, flush));
          }

          template<typename Append, typename Flush>
            delegating_fhglog_appender (Append append, Flush flush)
              : log::Appender ("event-handler")
              , _append (append)
              , _flush (flush)
          { }

          void append (const log::LogEvent& evt)
          {
            _append (evt);
          }

          void flush()
          {
            _flush();
          }

        private:
          boost::function<void (const log::LogEvent&)> _append;
          boost::function<void()> _flush;
        };
      }

      worker_model::worker_model (unsigned short port, QObject* parent)
        : QAbstractItemModel (parent)
        , _workers()
        , _worker_containers()
        , _base_time (QDateTime::currentDateTime())
        , _event_queue()
        , _queued_events()
        , _log_server ( delegating_fhglog_appender::create
                        ( boost::bind (&worker_model::append_event, this, _1)
                        , boost::bind (&worker_model::handle_events, this)
                        )
                      , _io_service
                      , port
                      )
        , _io_thread (boost::bind (&boost::asio::io_service::run, &_io_service))
      {
        QTimer* timer (new QTimer (this));
        connect (timer, SIGNAL (timeout()), SLOT (handle_events()));
        //! \note log::LogEvent::tstamp_t has resolution of seconds
        timer->start (100);
      }

      worker_model::~worker_model()
      {
        _io_service.stop();
        _io_thread.join();
      }

      void worker_model::append_event (const log::LogEvent& event)
      {
        boost::lock_guard<boost::mutex> guard (_event_queue);
        _queued_events << event;
      }

      void worker_model::handle_events()
      {
        boost::optional<QModelIndex> ul, br;

        QVector<log::LogEvent> events;
        {
          boost::lock_guard<boost::mutex> guard (_event_queue);
          std::swap (events, _queued_events);
        }

        BOOST_FOREACH (log::LogEvent log_event, events)
        {
          const sdpa::daemon::NotificationEvent event (log_event.message());

          const long time
            ( QDateTime::fromTime_t (log_event.tstamp()).toMSecsSinceEpoch()
            - _base_time.toMSecsSinceEpoch()
            );

          const QString activity_id
            (QString::fromStdString (event.activity_id()));
          const QString worker (QString::fromStdString (event.component()));

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
                         , boost::bind (&value_type::id, _1) == activity_id
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

        if (ul)
        {
          emit dataChanged (*ul, br ? *br : *ul);
        }
      }

      int worker_model::rowCount (const QModelIndex& parent) const
      {
        return parent.isValid() ? 0 : _workers.size();
      }
      int worker_model::columnCount (const QModelIndex& parent) const
      {
        return parent.isValid() ? 0 : 1;
      }

      namespace
      {
        bool is_before ( const worker_model::timestamp_type t
                       , const worker_model::value_type val
                       )
        {
          return !val.duration() || t < (val.timestamp() + *val.duration());
        }

        worker_model::subrange_type find_subrange
          ( const std::vector<worker_model::value_type>::const_iterator begin
          , const std::vector<worker_model::value_type>::const_iterator end
          , const worker_model::timestamp_type from
          , const worker_model::timestamp_type to
          )
        {
          const std::vector<worker_model::value_type>::const_iterator lower
            (std::upper_bound (begin, end, from, is_before));

          return worker_model::subrange_type
            ( lower
            , std::lower_bound
              ( lower, end
              , to, boost::bind (&worker_model::value_type::timestamp, _1) < _2
              )
            );
        }
      }

      QVariant worker_model::data (const QModelIndex& index, int role) const
      {
        if (index.isValid())
        {
          if (role == name_role || role == Qt::DisplayRole)
          {
            return _workers[index.row()];
          }
          else if (role == current_interval_role)
          {
            return QVariant::fromValue<boost::optional<value_type> >
              (_worker_containers[_workers[index.row()]].back());
          }
          else if (role == range_getter_role)
          {
            return QVariant::fromValue<subrange_getter_type>
              ( boost::bind
                ( &find_subrange
                , _worker_containers.find (_workers[index.row()])->begin()
                , _worker_containers.find (_workers[index.row()])->end()
                , _1
                , _2
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
        (int row, int column, const QModelIndex& parent) const
      {
        return !hasIndex (row, column, parent)
          ? QModelIndex()
          : createIndex (row, column, 0);
      }

      QModelIndex worker_model::parent (const QModelIndex&) const
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
        (int row, int count, const QModelIndex& parent)
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
