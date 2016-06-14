// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <fhglog/event.hpp>
#include <fhglog/remote/server.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/range.hpp>

#include <QAbstractItemModel>
#include <QDateTime>
#include <QVector>

#include <functional>
#include <mutex>
#include <thread>

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

        worker_model (unsigned short port, QObject* parent = nullptr);
        ~worker_model();

        virtual int rowCount (const QModelIndex& = QModelIndex()) const override;
        virtual int columnCount (const QModelIndex& = QModelIndex()) const override;
        virtual QVariant data
          (const QModelIndex&, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData
          (int section, Qt::Orientation, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index
          (int row, int column, const QModelIndex& = QModelIndex()) const override;
        virtual QModelIndex parent (const QModelIndex&) const override;

        void clear();
        virtual bool removeRows
          (int row, int count, const QModelIndex& parent = QModelIndex()) override;

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
        typedef std::function<subrange_type (timestamp_type, timestamp_type)>
          subrange_getter_type;

      private slots:
        void handle_events();

      private:
        QList<QString> _workers;
        QMap<QString, std::vector<value_type>> _worker_containers;
        QDateTime _base_time;

        std::mutex _event_queue;
        QVector<log::LogEvent> _queued_events;
        void append_event (const log::LogEvent&);

        boost::asio::io_service _io_service;
        fhg::log::Logger _logger;
        log::remote::LogServer _log_server;
        std::thread _io_thread;
      };
    }
  }
}

//! \note current_interval_role
Q_DECLARE_METATYPE (boost::optional<fhg::pnete::ui::worker_model::value_type>)
//! \note range_getter_role
Q_DECLARE_METATYPE (fhg::pnete::ui::worker_model::subrange_getter_type)
