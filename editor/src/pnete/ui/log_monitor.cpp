#include <pnete/ui/log_monitor.hpp>

#include <util/qt/boost_connect.hpp>

#include <we/loader/putget.hpp>
#include <we/type/value.hpp>
#include <we/type/net.hpp> // recursive wrapper of transition_t fails otherwise.
#include <we/mgmt/type/activity.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/access.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lambda/lambda.hpp>

#include <QAction>
#include <QApplication>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QGroupBox>
#include <QDial>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QSlider>
#include <QSplitter>
#include <QCheckBox>
#include <QComboBox>
#include <QTableView>
#include <QTime>
#include <QTimer>
#include <QString>
#include <QVBoxLayout>
#include <QMutexLocker>
#include <QThread>

#include <cmath>
#include <fstream>
#include <list>
#include <sstream>
#include <stdexcept>

namespace
{
  class function_call_appender : public fhg::log::Appender
  {
  public:
    typedef boost::function<void (const fhg::log::LogEvent&)> event_handler_t;

    function_call_appender (const event_handler_t& handler)
      : fhg::log::Appender ("event-handler")
      , _handler (handler)
    { }

    void append (const fhg::log::LogEvent& evt)
    {
      _handler (evt);
    }

    void flush()
    {
    }

  private:
    event_handler_t _handler;
  };

  template<typename T>
    fhg::log::Appender::ptr_t appender_with
    ( void (T::* function)(const fhg::log::LogEvent&)
    , T* that
    )
  {
    return fhg::log::Appender::ptr_t
      (new function_call_appender (boost::bind (function, that, _1)));
  }

  QColor severityToColor (const fhg::log::LogLevel lvl)
  {
    switch (lvl.lvl())
    {
    case fhg::log::LogLevel::TRACE:
      return QColor (205, 183, 158);
    case fhg::log::LogLevel::DEBUG:
      return QColor (139, 125, 107);
    case fhg::log::LogLevel::INFO:
      return QColor (25, 25, 25);
    case fhg::log::LogLevel::WARN:
      return QColor (255, 140, 0);
    case fhg::log::LogLevel::ERROR:
      return QColor (255, 0, 0);
    case fhg::log::LogLevel::FATAL:
      return QColor (165, 42, 42);
    default:
      return QColor (0, 0, 0);
    }
  }

  enum table_columns
  {
    TABLE_COL_TIME,
    TABLE_COL_SOURCE,
    TABLE_COL_LOCATION,
    TABLE_COL_MESSAGE,
    TABLE_COLUMN_COUNT,
  };
}

namespace detail
{
  formatted_log_event::formatted_log_event (const fhg::log::LogEvent& evt)
    //! \todo get time from outside?
    : time (QTime::currentTime().toString())
    , source (QString ("%1@%2").arg (evt.pid()).arg (evt.logged_on().c_str()))
    , location (QString ("%1:%2").arg (evt.file().c_str()).arg (evt.line()))
    , message (evt.message().c_str())
    , event (evt)
  { }

  log_table_model::log_table_model (QObject* parent)
    : QAbstractTableModel (parent)
    , _pending_data()
    , _clear_on_update()
    , _mutex_pending (QMutex::Recursive)
    , _data()
    , _mutex_data (QMutex::Recursive)
  { }

  int log_table_model::rowCount (const QModelIndex&) const
  {
    QMutexLocker lock (&_mutex_data);
    return _data.size();
  }

  int log_table_model::columnCount (const QModelIndex&) const
  {
    return TABLE_COLUMN_COUNT;
  }

  QVariant log_table_model::headerData
    (int section, Qt::Orientation orientation, int role) const
  {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
      switch (section)
      {
      case TABLE_COL_TIME:
        static const QString time (tr ("Time"));
        return time;
      case TABLE_COL_SOURCE:
        static const QString source (tr ("Source"));
        return source;
      case TABLE_COL_LOCATION:
        static const QString location (tr ("Location"));
        return location;
      case TABLE_COL_MESSAGE:
        static const QString message (tr ("Message"));
        return message;
      }
    }

    return QVariant();
  }

  QVariant log_table_model::data (const QModelIndex& index, int role) const
  {
    QMutexLocker lock (&_mutex_data);

    const formatted_log_event& event (_data[index.row()]);

    switch (role)
    {
    case Qt::DisplayRole:
      switch (index.column())
      {
      case TABLE_COL_TIME:
        return event.time;
      case TABLE_COL_SOURCE:
        return event.source;
      case TABLE_COL_LOCATION:
        return event.location;
      case TABLE_COL_MESSAGE:
        return event.message;
      }

      break;

    case Qt::ForegroundRole:
      return severityToColor (event.event.severity());

    case Qt::UserRole:
      return static_cast<int> (event.event.severity());
    }

    return QVariant();
  }

  std::vector<fhg::log::LogEvent> log_table_model::data() const
  {
    QMutexLocker lock (&_mutex_data);

    std::vector<fhg::log::LogEvent> result;

    foreach (const formatted_log_event& event, _data)
    {
      result.push_back (event.event);
    }

    return result;
  }

  void log_table_model::add (const fhg::log::LogEvent& event)
  {
    QMutexLocker lock (&_mutex_pending);
    _pending_data.push_back (formatted_log_event (event));
  }

  void log_table_model::clear()
  {
    QMutexLocker lock (&_mutex_pending);
    _clear_on_update = true;
  }

  void log_table_model::update()
  {
    //! \note These operations _need_ to be exclusive during one event
    //! loop, as Qt emits some signals which will fuck everything up,
    //! if you do stuff like adding and removing rows at once.

    //! \note emulate circular buffer
    {
      QMutexLocker lock (&_mutex_data);

      //! \todo Configurable limit.
      static const int limit (10000);
      static const int to_be_removed (limit * 0.1);
      if (_data.size() > limit)
      {
        const int really_removing (std::min (to_be_removed, _data.size()));

        beginRemoveRows (QModelIndex(), 0, really_removing - 1);
        _data.erase (_data.begin(), _data.begin() + really_removing);
        endRemoveRows();

        return;
      }
    }

    //! \note Clear also removes pending data, which is fine. This
    //! saves us from locking twice.
    QList<formatted_log_event> pending_data;
    bool clear_on_update (false);

    {
      QMutexLocker lock (&_mutex_pending);
      std::swap (pending_data, _pending_data);
      std::swap (clear_on_update, _clear_on_update);
    }

    if (clear_on_update && _data.size())
    {
      QMutexLocker lock (&_mutex_data);

      beginRemoveRows (QModelIndex(), 0, _data.size() - 1);
      _data.clear();
      endRemoveRows();
    }
    else if (!pending_data.isEmpty())
    {
      QMutexLocker lock (&_mutex_data);

      beginInsertRows
        (QModelIndex(), _data.size(), _data.size() + pending_data.size() - 1);
      _data.append (pending_data);
      endInsertRows();
    }
  }

  class log_filter_proxy : public QSortFilterProxyModel
  {
  public:
     log_filter_proxy (QObject* parent = NULL)
       : QSortFilterProxyModel (parent)
    { }

    void minimum_severity (int minimum_severity_)
    {
      _minimum_severity = minimum_severity_;
      invalidateFilter();
    }

  protected:
    virtual bool filterAcceptsRow (int row, const QModelIndex& parent) const
    {
      return sourceModel()->data
        (sourceModel()->index (row, 0, parent), Qt::UserRole).toInt()
        >= _minimum_severity;
    }

 private:
    int _minimum_severity;
 };
}


log_monitor::log_monitor (unsigned short port, QWidget* parent)
  : QWidget (parent)
  , _drop_filtered (true)
  , _filter_level (2)
  , _log_table (new QTableView (this))
  , _log_model (new detail::log_table_model)
  , _log_filter (new detail::log_filter_proxy (this))
  //! \todo Do updates in separate thread again?
  // , _log_model_update_thread (new QThread (this))
  , _log_model_update_timer (new QTimer (this))
  , _io_service()
  , _log_server
    (appender_with (&log_monitor::append_log_event, this), _io_service, port)
  , _io_thread (boost::bind (&boost::asio::io_service::run, &_io_service))
{
  // _log_model->moveToThread (_log_model_update_thread);
  connect ( _log_model_update_timer, SIGNAL (timeout())
          , _log_model, SLOT (update())
          );
  // _log_model_update_thread->start();
  _log_model_update_timer->start();

  _log_filter->setDynamicSortFilter (true);
  _log_filter->setSourceModel (_log_model);
  _log_table->setModel (_log_filter);

  _log_table->setAlternatingRowColors (false);
  _log_table->setHorizontalScrollMode (QAbstractItemView::ScrollPerPixel);
  _log_table->setSelectionMode (QAbstractItemView::NoSelection);
  _log_table->setShowGrid (false);
  _log_table->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
  _log_table->setWordWrap (false);
  _log_table->verticalHeader()->setVisible (false);
  _log_table->horizontalHeader()->setStretchLastSection (true);

  QGroupBox* filter_level_box (new QGroupBox (tr ("Filter"), this));

  QDial* filter_level_dial (new QDial (this));
  filter_level_dial->setOrientation (Qt::Horizontal);

  QComboBox* filter_level_combobox (new QComboBox (this));
  filter_level_combobox->setToolTip (tr ("Filter events according to level"));

  filter_level_dial->setMaximum (5);
  filter_level_combobox->insertItems ( 0
                                       , QStringList()
                                       << tr ("Trace")
                                       << tr ("Debug")
                                       << tr ("Info")
                                       << tr ("Warn")
                                       << tr ("Error")
                                       << tr ("Fatal")
                                       );

  fhg::util::qt::boost_connect<void (int)>
    ( filter_level_combobox, SIGNAL (currentIndexChanged(int))
    , this, boost::lambda::var (_filter_level) = boost::lambda::_1
    );
  fhg::util::qt::boost_connect<void (int)>
    ( filter_level_combobox
    , SIGNAL (currentIndexChanged(int))
    , _log_filter
    , boost::bind (&detail::log_filter_proxy::minimum_severity, _log_filter, _1)
    );

  connect ( filter_level_dial, SIGNAL (valueChanged(int))
          , filter_level_combobox, SLOT (setCurrentIndex(int))
          );
  connect ( filter_level_combobox, SIGNAL (currentIndexChanged(int))
          , filter_level_dial, SLOT (setValue(int))
          );

  filter_level_combobox->setCurrentIndex (_filter_level);

  QCheckBox* drop_filtered_box (new QCheckBox (tr ("drop filtered"), this));
  drop_filtered_box->setChecked (_drop_filtered);
  drop_filtered_box->setToolTip
    (tr ("Drop filtered events instead of keeping them"));
  fhg::util::qt::boost_connect<void (bool)>
    ( drop_filtered_box, SIGNAL (toggled (bool))
    , this, boost::lambda::var (_drop_filtered) = boost::lambda::_1
    );


  QGroupBox* control_box (new QGroupBox (tr ("Control"), this));

  QPushButton* clear_log_button (new QPushButton (tr ("Clear"), this));
  clear_log_button->setToolTip (tr ("Clear all events"));
  fhg::util::qt::boost_connect<void()>
    ( clear_log_button, SIGNAL (clicked())
    , _log_model, boost::bind (&detail::log_table_model::clear, _log_model)
    );

  QCheckBox* follow_logging_cb (new QCheckBox (tr ("follow"), this));
  follow_logging_cb->setToolTip ( tr ( "Follow the stream of log events and "
                                       "automatically scroll the view, drop "
                                       "events otherwise"
                                     )
                                );
  connect ( follow_logging_cb, SIGNAL (toggled (bool))
          , this, SLOT (toggle_follow_logging (bool))
          );
  follow_logging_cb->setChecked (true);


  QVBoxLayout* log_filter_layout (new QVBoxLayout (filter_level_box));
  log_filter_layout->addWidget (filter_level_dial);
  log_filter_layout->addWidget (filter_level_combobox);
  log_filter_layout->addWidget (drop_filtered_box);

  QVBoxLayout* control_box_layout (new QVBoxLayout (control_box));
  control_box_layout->addWidget (follow_logging_cb);
  control_box_layout->addWidget (clear_log_button);

  QVBoxLayout* log_sidebar_layout (new QVBoxLayout);
  log_sidebar_layout->addWidget (filter_level_box);
  log_sidebar_layout->addStretch();
  log_sidebar_layout->addWidget (control_box);

  QGridLayout* layout (new QGridLayout (this));
  layout->addWidget (_log_table, 0, 0);
  layout->addLayout (log_sidebar_layout, 0, 1);


  QAction* save_log (new QAction (tr ("save_log"), this));
  save_log->setShortcuts (QKeySequence::Save);
  connect (save_log, SIGNAL (triggered()), this, SLOT (save()));
  addAction (save_log);
}

log_monitor::~log_monitor()
{
  _io_service.stop();
  _io_thread.join();

  _log_model_update_timer->stop();
  // _log_model_update_thread->quit();
  // _log_model_update_thread->wait();
}

void log_monitor::append_log_event (const fhg::log::LogEvent & evt)
{
  if (evt.severity() >= _filter_level || !_drop_filtered)
  {
    _log_model->add (evt);
  }
}

void log_monitor::toggle_follow_logging (bool follow)
{
  if (!follow)
  {
    return;
  }

  _log_table->scrollToBottom();

  QTimer* log_follower (new QTimer (this));

  connect (log_follower, SIGNAL (timeout()), _log_table, SLOT (scrollToBottom()));
  connect (sender(), SIGNAL (toggled (bool)), log_follower, SLOT (stop()));
  connect (sender(), SIGNAL (toggled (bool)), log_follower, SLOT (deleteLater()));

  //! \todo Configurable refresh rate.
  static const int refresh_rate (0 /*ms*/);
  log_follower->start (refresh_rate);
}

void log_monitor::save ()
{
  const QString fname
    (QFileDialog::getSaveFileName (this, "Save log messages", _last_saved_filename));

  if (fname.isEmpty ())
    return;

  try
  {
    std::ofstream ofs (fname.toStdString ().c_str ());
    boost::archive::text_oarchive oa (ofs);
    std::vector<fhg::log::LogEvent> data (_log_model->data());
    oa & data;
    _last_saved_filename = fname;
  }
  catch (std::exception const & ex)
  {
    QMessageBox::critical (this, "Could not save file", ex.what ());
  }
}
