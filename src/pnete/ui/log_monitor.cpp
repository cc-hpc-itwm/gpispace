#include <pnete/ui/log_monitor.hpp>

#include <we/type/activity.hpp>

#include <util-generic/ostream/put_time.hpp>
#include <util-generic/this_bound_mem_fn.hpp>

#include <util-qt/overload.hpp>

#include <boost/filesystem.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

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

#include <algorithm>
#include <exception>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace
{
  struct category_data_t
  {
    QColor color;
    int legacy_level;
    char legacy_level_identifier;
  };

  category_data_t category_to_data (std::string const& category)
  {
#define ENTRY(name_, color_, level_, identifier_)      \
      { fhg::logging::legacy::category_level_ ## name_   \
      , category_data_t {color_, level_, identifier_}    \
      }

    static std::unordered_map<std::string, category_data_t> const category_data
      { ENTRY (trace, QColor (205, 183, 158), 0, 'T')
      , ENTRY (info, QColor (25, 25, 25), 1, 'I')
      , ENTRY (warn, QColor (255, 140, 0), 2, 'W')
      , ENTRY (error, QColor (255, 0, 0), 3, 'E')
      };

#undef ENTRY

    auto it (category_data.find (category));
    return it != category_data.end() ? it->second
      : throw std::invalid_argument ("category is not a legacy category");
  }

  QColor category_to_color (std::string const& category)
  {
    return category_to_data (category).color;
  }
  int category_to_legacy_level (std::string const& category)
  {
    return category_to_data (category).legacy_level;
  }
  char category_to_short_legacy_level_identifier (std::string const& category)
  {
    return category_to_data (category).legacy_level_identifier;
  }

  enum table_columns
  {
    TABLE_COL_TIME,
    TABLE_COL_SOURCE,
    TABLE_COL_MESSAGE,
    TABLE_COLUMN_COUNT,
  };

  template<typename Timepoint>
    QString timepoint_to_qstring (Timepoint const& timepoint)
  {
    using put_time = fhg::util::ostream::put_time<typename Timepoint::clock>;
    return QString::fromStdString (put_time (timepoint).string());
  }
}

namespace detail
{
  formatted_log_event::formatted_log_event (fhg::logging::message raw)
    : time (timepoint_to_qstring (raw._timestamp))
    , source ( QString ("%1@%2")
               .arg (raw._process_id)
               .arg (QString::fromStdString (raw._hostname))
             )
    , message (QString::fromStdString (raw._content))
    , color (category_to_color (raw._category))
    , legacy_severity (category_to_legacy_level (raw._category))
    , _raw (std::move (raw))
  {}

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
      case TABLE_COL_MESSAGE:
        return event.message;
      }

      break;

    case Qt::ForegroundRole:
      return event.color;

    case Qt::UserRole:
      return event.legacy_severity;
    }

    return QVariant();
  }

  std::vector<fhg::logging::message> log_table_model::data() const
  {
    QMutexLocker lock (&_mutex_data);

    std::vector<fhg::logging::message> result;

    for (const formatted_log_event& event : _data)
    {
      result.push_back (event._raw);
    }

    return result;
  }

  void log_table_model::add (fhg::logging::message message)
  {
    QMutexLocker lock (&_mutex_pending);
    _pending_data.push_back (std::move (message));
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
     log_filter_proxy (QObject* parent = nullptr)
       : QSortFilterProxyModel (parent)
    { }

    void minimum_severity (int minimum_severity_)
    {
      _minimum_severity = minimum_severity_;
      invalidateFilter();
    }

  protected:
    virtual bool filterAcceptsRow (int row, const QModelIndex& parent) const override
    {
      return sourceModel()->data
        (sourceModel()->index (row, 0, parent), Qt::UserRole).toInt()
        >= _minimum_severity;
    }

 private:
    int _minimum_severity;
 };
}


log_monitor::log_monitor()
  : QWidget()
  , _drop_filtered (false)
  , _filter_level (1)
  , _log_table (new QTableView (this))
  , _log_model (new detail::log_table_model)
  , _log_filter (new detail::log_filter_proxy (this))
  //! \todo Do updates in separate thread again?
  // , _log_model_update_thread (new QThread (this))
  , _log_model_update_timer (new QTimer (this))
{
  // _log_model->moveToThread (_log_model_update_thread);
  connect ( _log_model_update_timer, SIGNAL (timeout())
          , _log_model, SLOT (update())
          );
  // _log_model_update_thread->start();
  _log_model_update_timer->start (20 /*ms*/);

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
  _log_table->verticalHeader()->setSectionResizeMode
    (QHeaderView::ResizeToContents);

  QGroupBox* filter_level_box (new QGroupBox (tr ("Filter"), this));

  QDial* filter_level_dial (new QDial (this));
  filter_level_dial->setOrientation (Qt::Horizontal);

  QComboBox* filter_level_combobox (new QComboBox (this));
  filter_level_combobox->setToolTip (tr ("Filter events according to level"));

  filter_level_dial->setMaximum (4);
  filter_level_combobox->insertItems ( 0
                                       , QStringList()
                                       << tr ("Trace")
                                       << tr ("Info")
                                       << tr ("Warn")
                                       << tr ("Error")
                                       );

  connect
    ( filter_level_combobox, QOverload<int>::of (&QComboBox::currentIndexChanged)
    , this, boost::lambda::var (_filter_level) = boost::lambda::_1
    );
  connect
    ( filter_level_combobox
    , QOverload<int>::of (&QComboBox::currentIndexChanged)
    , _log_filter
    , std::bind (&detail::log_filter_proxy::minimum_severity, _log_filter, std::placeholders::_1)
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
  connect
    ( drop_filtered_box, &QCheckBox::toggled
    , this, boost::lambda::var (_drop_filtered) = boost::lambda::_1
    );


  QGroupBox* control_box (new QGroupBox (tr ("Control"), this));

  QPushButton* clear_log_button (new QPushButton (tr ("Clear"), this));
  clear_log_button->setToolTip (tr ("Clear all events"));
  connect
    ( clear_log_button, &QPushButton::clicked
    , _log_model, std::bind (&detail::log_table_model::clear, _log_model)
    );

  QCheckBox* follow_logging_cb (new QCheckBox (tr ("follow"), this));
  follow_logging_cb->setToolTip ( tr ( "Follow the stream of log events and "
                                       "automatically scroll the view"
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
}

log_monitor::~log_monitor()
{
  _log_model_update_timer->stop();
  // _log_model_update_thread->quit();
  // _log_model_update_thread->wait();
}

void log_monitor::append_log_event (fhg::logging::message const& message)
{
  if (message._category == sdpa::daemon::gantt_log_category)
  {
    return;
  }

  if ( category_to_legacy_level (message._category) >= _filter_level
     || !_drop_filtered
     )
  {
    _log_model->add (std::move (message));
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
  static const int refresh_rate (20 /*ms*/);
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

    for (auto const& message : _log_model->data())
    {
      ofs << "["
          << fhg::util::ostream::put_time<decltype (message._timestamp)::clock>
               (message._timestamp)
          << "] "
          << message._hostname
          << ": "
          << "pid "
          << message._process_id
          << " tid"
          << message._thread_id
          << ": "
          << category_to_short_legacy_level_identifier (message._category)
          << ": "
          << message._content
          << "\n";
    }
    _last_saved_filename = fname;
  }
  catch (std::exception const & ex)
  {
    QMessageBox::critical (this, "Could not save file", ex.what ());
  }
}
