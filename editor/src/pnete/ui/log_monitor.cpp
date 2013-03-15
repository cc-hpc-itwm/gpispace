#include <pnete/ui/log_monitor.hpp>

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
      , m_handler (handler)
    { }

    void append (const fhg::log::LogEvent& evt)
    {
      m_handler (evt);
    }

    void flush()
    {
    }

  private:
    event_handler_t m_handler;
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
  class log_table_model : public QAbstractTableModel
  {
  public:
    log_table_model (QObject* parent = NULL)
      : QAbstractTableModel (parent)
    { }

    virtual int rowCount (const QModelIndex& = QModelIndex()) const
    {
      return _data.size();
    }

    virtual int columnCount (const QModelIndex& = QModelIndex()) const
    {
      return TABLE_COLUMN_COUNT;
    }

    virtual QVariant headerData ( int section
                                , Qt::Orientation orientation
                                , int role = Qt::DisplayRole
                                ) const
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

    virtual QVariant
      data (const QModelIndex& index, int role = Qt::DisplayRole) const
    {
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

    void clear()
    {
      beginResetModel();
      _data.clear();
      endResetModel();
    }

    void add (const fhg::log::LogEvent& event)
    {
      beginInsertRows (QModelIndex(), rowCount() - 1, rowCount() - 1);
      _data.push_back (formatted_log_event (event));
      endInsertRows();
    }

  private:
    struct formatted_log_event
    {
      QString time;
      QString source;
      QString location;
      QString message;

      fhg::log::LogEvent event;

      formatted_log_event (const fhg::log::LogEvent& evt)
        //! \todo get time from outside?
        : time (QTime::currentTime().toString())
        , source (QString ("%1@%2").arg (evt.pid()).arg (evt.logged_on().c_str()))
        , location (QString ("%1:%2").arg (evt.file().c_str()).arg (evt.line()))
        , message (evt.message().c_str())
        , event (evt)
      { }
    };

    QList<formatted_log_event> _data;
  };

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
  , m_log_server ( appender_with (&log_monitor::handle_external_event, this)
                 , m_io_service
                 , port
                 )
  , m_io_thread (boost::bind (&boost::asio::io_service::run, &m_io_service))
  , m_drop_filtered (new QCheckBox (tr ("drop filtered"), this))
  , m_level_filter_selector (new QComboBox (this))
  , m_log_table (new QTableView (this))
  , _log_model (new detail::log_table_model (this))
  , _log_filter (new detail::log_filter_proxy (this))
{
  _log_filter->setDynamicSortFilter (true);
  _log_filter->setSourceModel (_log_model);
  m_log_table->setModel (_log_filter);

  m_log_table->setAlternatingRowColors (false);
  m_log_table->setHorizontalScrollMode (QAbstractItemView::ScrollPerPixel);
  m_log_table->setSelectionMode (QAbstractItemView::NoSelection);
  m_log_table->setShowGrid (false);
  m_log_table->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
  m_log_table->setWordWrap (false);
  m_log_table->verticalHeader()->setVisible (false);
  m_log_table->horizontalHeader()->setStretchLastSection (true);

  QGroupBox* log_filter_box (new QGroupBox (tr ("Filter"), this));

  QDial* log_filter_dial (new QDial (this));
  log_filter_dial->setOrientation (Qt::Horizontal);

  m_level_filter_selector->setToolTip (tr ("Filter events according to level"));

  log_filter_dial->setMaximum (5);
  m_level_filter_selector->insertItems ( 0
                                       , QStringList()
                                       << tr ("Trace")
                                       << tr ("Debug")
                                       << tr ("Info")
                                       << tr ("Warn")
                                       << tr ("Error")
                                       << tr ("Fatal")
                                       );

  connect ( m_level_filter_selector, SIGNAL (currentIndexChanged(int))
          , this, SLOT (levelFilterChanged(int))
          );

  connect ( log_filter_dial, SIGNAL (valueChanged(int))
          , m_level_filter_selector, SLOT (setCurrentIndex(int))
          );
  connect ( m_level_filter_selector, SIGNAL (currentIndexChanged(int))
          , log_filter_dial, SLOT (setValue(int))
          );

  m_level_filter_selector->setCurrentIndex (2);

  m_drop_filtered->setCheckState (Qt::Checked);
  m_drop_filtered->setToolTip
    (tr ("Drop filtered events instead of keeping them"));


  QGroupBox* control_box (new QGroupBox (tr ("Control"), this));

  QPushButton* clear_log_button (new QPushButton (tr ("Clear"), this));
  clear_log_button->setToolTip (tr ("Clear all events"));
  connect (clear_log_button, SIGNAL (clicked()), this, SLOT (clearLogging()));

  QCheckBox* follow_logging_cb (new QCheckBox (tr ("follow"), this));
  follow_logging_cb->setToolTip ( tr ( "Follow the stream of log events and "
                                       "automatically scroll the view, drop "
                                       "events otherwise"
                                     )
                                );
  connect ( follow_logging_cb, SIGNAL (toggled (bool))
          , this, SLOT (toggleFollowLogging (bool))
          );
  follow_logging_cb->setChecked (true);


  QVBoxLayout* log_filter_layout (new QVBoxLayout (log_filter_box));
  log_filter_layout->addWidget (log_filter_dial);
  log_filter_layout->addWidget (m_level_filter_selector);
  log_filter_layout->addWidget (m_drop_filtered);

  QVBoxLayout* control_box_layout (new QVBoxLayout (control_box));
  control_box_layout->addWidget (follow_logging_cb);
  control_box_layout->addWidget (clear_log_button);

  QVBoxLayout* log_sidebar_layout (new QVBoxLayout);
  log_sidebar_layout->addWidget (log_filter_box);
  log_sidebar_layout->addStretch();
  log_sidebar_layout->addWidget (control_box);

  QGridLayout* layout (new QGridLayout (this));
  layout->addWidget (m_log_table, 0, 0);
  layout->addLayout (log_sidebar_layout, 0, 1);


  QAction* save_log (new QAction (tr ("save_log"), this));
  save_log->setShortcuts (QKeySequence::Save);
  connect (save_log, SIGNAL (triggered()), this, SLOT (save()));
  addAction (save_log);
}

log_monitor::~log_monitor()
{
  m_io_service.stop();
  m_io_thread.join();
}

namespace
{
  class fhglog_event : public QEvent
  {
  public:
    fhglog_event (const fhg::log::LogEvent& e)
      : QEvent (QEvent::User)
      , log_event (e)
    { }

    fhg::log::LogEvent log_event;
  };
}

void log_monitor::handle_external_event (const fhg::log::LogEvent & evt)
{
  QApplication::postEvent (this, new fhglog_event (evt));
}

bool log_monitor::event (QEvent* event)
{
  if (event->type() == QEvent::User)
  {
    event->accept();

    const fhg::log::LogEvent& evt (static_cast<fhglog_event*> (event)->log_event);

    if ( !( evt.severity() < m_level_filter_selector->currentIndex()
          && m_drop_filtered->isChecked()
          )
       )
    {
      // TODO:
      //    maximum number of events (circular buffer like)

      _log_model->add (evt);
      m_log_events.push_back (evt);
    }

    return true;
  }

  return QWidget::event (event);
}

void log_monitor::clearLogging ()
{
  m_log_events.clear();
  _log_model->clear();
}

void log_monitor::toggleFollowLogging (bool follow)
{
  if (!follow)
  {
    return;
  }

  m_log_table->scrollToBottom();

  QTimer* log_follower (new QTimer (this));

  connect (log_follower, SIGNAL (timeout()), m_log_table, SLOT (scrollToBottom()));
  connect (sender(), SIGNAL (toggled (bool)), log_follower, SLOT (stop()));
  connect (sender(), SIGNAL (toggled (bool)), log_follower, SLOT (deleteLater()));

  static const int refresh_rate (200 /*ms*/);
  log_follower->start (refresh_rate);
}

void log_monitor::levelFilterChanged (int lvl)
{
  _log_filter->minimum_severity (lvl);
}

void log_monitor::save ()
{
  QString fname = QFileDialog::getSaveFileName ( this
                                               , "Save log messages"
                                               , m_logfile
                                               );

  if (fname.isEmpty ())
    return;

  try
  {
    std::ofstream ofs (fname.toStdString ().c_str ());
    boost::archive::text_oarchive oa (ofs);
    oa & m_log_events;
    m_logfile = fname;
  }
  catch (std::exception const & ex)
  {
    QMessageBox::critical (this, "Could not save file", ex.what ());
  }
}
