// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/execution_monitor_worker_model.hpp>

#include <fhg/assert.hpp>

#include <QTimer>

#include <functional>
#include <string>


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
        case sdpa::daemon::NotificationEvent::STATE_VMEM_PUT_FINISHED:
        case sdpa::daemon::NotificationEvent::STATE_VMEM_GET_FINISHED:
          duration (now - timestamp());
          break;

        case sdpa::daemon::NotificationEvent::STATE_STARTED:
          break;
        }

        _state = state_;
      }

      namespace
      {
        struct delegating_fhglog_appender : public log::Appender
        {
          template<typename Append, typename Flush>
          static fhg::log::Logger& create
            ( Append append
            , Flush flush
            , fhg::log::Logger& logger
            )
          {
            logger.addAppender<delegating_fhglog_appender> (append, flush);

            logger.setLevel (fhg::log::TRACE);

            return logger;
          }

          template<typename Append, typename Flush>
            delegating_fhglog_appender (Append append, Flush flush)
              : _append (append)
              , _flush (flush)
          { }

          virtual void append (const log::LogEvent& evt) override
          {
            _append (evt);
          }

          virtual void flush() override
          {
            _flush();
          }

        private:
          std::function<void (const log::LogEvent&)> _append;
          std::function<void()> _flush;
        };
      }

      worker_model::worker_model (unsigned short port, boost::optional<boost::filesystem::path> const& trace_file, QObject* parent)
        : QAbstractItemModel (parent)
        , _workers()
        , _worker_containers()
        , _base_time (QDateTime::currentDateTime())
        , _event_queue()
        , _queued_events()
        , _io_service()
        , _logger()
        , _log_server ( delegating_fhglog_appender::create
                        ( std::bind (&worker_model::append_event, this, std::placeholders::_1)
                        , std::bind (&worker_model::handle_events, this)
                        , _logger
                        )
                      , _io_service
                      , port
                      )
        , _io_thread ([this] { _io_service.run(); })
        , trace_appender(trace_file
                          ? boost::optional<fhg::log::SWFTraceAppender>(trace_file.value())
                          : boost::none)
      {

        QTimer* timer (new QTimer (this));
        connect (timer, SIGNAL (timeout()), SLOT (handle_events()));
        //! \note log::LogEvent::tstamp_t has resolution of seconds
        timer->start (100);

        if (trace_appender) {
          trace_appender.value().append_header(fhg::log::SWFTraceEvent::gen_swf_trace_header());
        }
      }

      worker_model::~worker_model()
      {
        _io_service.stop();
        _io_thread.join();
      }

      void worker_model::append_event (const log::LogEvent& event)
      {
        std::lock_guard<std::mutex> guard (_event_queue);
        _queued_events << event;
      }

      void worker_model::add_event_to_trace(const value_type& monitor_event,
          const sdpa::daemon::NotificationEvent& event,
          const std::string& worker_id_str)
      {
        if (trace_appender)
        {
          using sec_duration = std::chrono::duration<double, std::chrono::seconds::period>;
          using msec_duration = std::chrono::duration<double, std::chrono::milliseconds::period>;

          unsigned long job_id = std::hash<std::string>{}(event.activity_id());
          unsigned long worker_id = std::hash<std::string>{}(worker_id_str);
          we::transition_id_type trans_id = event.activity_transition_id().get_value_or(0);

          sec_duration start_ts_s = msec_duration(monitor_event.timestamp()); // event timestamp (job start time) is the duration in ms since _base_time
          sec_duration end_ts_s = start_ts_s + msec_duration(monitor_event.duration().value_or(0));

          sec_duration submission_ts_s = start_ts_s;
          if (event.activity_submission_ts())
          {
            // adjust timestamp relative to _base_time
            submission_ts_s = sec_duration(event.activity_submission_ts().get()) - msec_duration(_base_time.toMSecsSinceEpoch());
          }

          fhg::log::SWFTraceEvent trace_ev(job_id,
            submission_ts_s.count(),
            start_ts_s.count(),
            end_ts_s.count(),
            fhg::log::SWFTraceEvent::get_state(monitor_event.state()),
            trans_id.value(),
            fhg::log::SWFTraceEvent::get_job_type_id(event.activity_id()),
            worker_id);
          trace_appender.value().append(trace_ev);
        }
      }

      void worker_model::handle_events()
      {
        boost::optional<QModelIndex> ul, br;

        QVector<log::LogEvent> events;
        {
          std::lock_guard<std::mutex> guard (_event_queue);
          std::swap (events, _queued_events);
        }

        for (log::LogEvent log_event : events)
        {
          const sdpa::daemon::NotificationEvent event (log_event.message());
          const long time
            ( log_event.tstamp() * 1000.0
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
                  add_event_to_trace(current, event, worker_std);
                }
              }

              // current worker's container is empty - add the first event
              container.push_back
                ( value_type ( time         // duration in ms since the _base_time
                             , boost::none
                             , activity_id
                             , QString::fromStdString (event.activity_name())
                             , event.activity_state()
                             )
                );
            }
            else
            {
              // add new event in the container
              container.back().state (event.activity_state(), time);
              add_event_to_trace(container.back(), event, worker_std);
            }

            (ul ? br : ul) = index (row, 0);
          }
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
        (int row, int column, const QModelIndex& parent) const
      {
        return !hasIndex (row, column, parent)
          ? QModelIndex()
          : createIndex (row, column, nullptr);
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
