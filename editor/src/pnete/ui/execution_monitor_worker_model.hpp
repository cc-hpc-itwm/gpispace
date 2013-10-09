// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_UI_EXECUTION_MONITOR_WORKER_MODEL_HPP
#define FHG_PNETE_UI_EXECUTION_MONITOR_WORKER_MODEL_HPP

#include <fhglog/LogEvent.hpp>
#include <fhglog/remote/LogServer.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/range.hpp>
#include <boost/thread.hpp>

#include <QAbstractItemModel>
#include <QDateTime>
#include <QVector>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class worker_model : public QAbstractItemModel
      {
        Q_OBJECT

      public:
        enum roles
        {
          name_role = Qt::UserRole,
          base_time_role,
          current_interval_role,
          range_getter_role
        };

        worker_model (unsigned short port, QObject* parent = NULL);
        ~worker_model();

        virtual int rowCount (const QModelIndex& = QModelIndex()) const;
        virtual int columnCount (const QModelIndex& = QModelIndex()) const;
        virtual QVariant data
          (const QModelIndex&, int role = Qt::DisplayRole) const;
        virtual QVariant headerData
          (int section, Qt::Orientation, int role = Qt::DisplayRole) const;
        virtual QModelIndex index
          (int row, int column, const QModelIndex& = QModelIndex()) const;
        virtual QModelIndex parent (const QModelIndex&) const;

        typedef long timestamp_type;
        typedef long duration_type;
        typedef sdpa::daemon::NotificationEvent::state_t state_type;

        struct value_type
        {
          value_type ( timestamp_type, boost::optional<duration_type>
                     , QString id, QString name, state_type
                     );

          timestamp_type timestamp() const;
          boost::optional<duration_type> duration() const;

          QString id() const;

          state_type state() const;

        private:
          void duration (boost::optional<duration_type> d);
          void state (state_type state_, timestamp_type now);

          timestamp_type _timestamp;
          boost::optional<duration_type> _duration;

          QString _id;
          QString _name;
          state_type _state;

          friend class worker_model;
        };

        typedef boost::iterator_range<std::vector<value_type>::const_iterator>
          subrange_type;
        typedef boost::function<subrange_type (timestamp_type, timestamp_type)>
          subrange_getter_type;

      private slots:
        void handle_events();

      private:
        QList<QString> _workers;
        QMap<QString, std::vector<value_type> > _worker_containers;
        QDateTime _base_time;

        boost::mutex _event_queue;
        QVector<log::LogEvent> _queued_events;
        void append_event (const log::LogEvent&);

        boost::asio::io_service _io_service;
        log::remote::LogServer _log_server;
        boost::thread _io_thread;
      };
    }
  }
}

//! \note current_interval_role
Q_DECLARE_METATYPE (boost::optional<fhg::pnete::ui::worker_model::value_type>)
//! \note range_getter_role
Q_DECLARE_METATYPE (fhg::pnete::ui::worker_model::subrange_getter_type)

#endif
