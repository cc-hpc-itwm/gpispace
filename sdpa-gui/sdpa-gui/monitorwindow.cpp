#include "monitorwindow.hpp"
#include "task.h"

#include <fhglog/Appender.hpp>

//! \todo eliminate this include (that completes type transition_t::data)
#include <we/loader/putget.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/type/net.hpp>
#include <we/type/value.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/access.hpp>
#include <boost/tokenizer.hpp>

#include <QApplication>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QScrollBar>
#include <QSlider>
#include <QSplitter>
#include <QVBoxLayout>

#include <cmath>
#include <fstream>
#include <list>
#include <sstream>

using namespace std;
using namespace boost;

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
}

MonitorWindow::MonitorWindow( unsigned short exe_port
                            , unsigned short log_port
                            , QWidget *parent
                            )
  : QMainWindow (parent)
  , m_log_server ( appender_with (&MonitorWindow::handle_external_event, this)
                 , m_io_service
                 , log_port
                 )
  , m_exe_server ( appender_with (&MonitorWindow::append_exe, this)
                 , m_io_service
                 , exe_port
                 )
  , m_io_thread (boost::bind (&boost::asio::io_service::run, &m_io_service))
  , m_follow_logging (true)
  , m_follow_execution (true)
  , m_scene (new QGraphicsScene (this))
  , m_view (new QGraphicsView (m_scene))
  , m_component_scene (new QGraphicsScene (this))
  , m_component_view (new QGraphicsView (m_component_scene))
  , m_current_scale (1.0)
  , m_drop_filtered (new QCheckBox (tr ("drop filtered"), this))
  , m_level_filter_selector (new QComboBox (this))
  , m_log_table (new QTableWidget (this))
{
  QTabWidget* tab_widget (new QTabWidget (this));
  tab_widget->setEnabled (true);
  tab_widget->setAutoFillBackground (true);
  tab_widget->setDocumentMode (false);

  // --- logging tab
  {
    QWidget* logging_tab (new QWidget (this));

    m_log_table->setAlternatingRowColors (false);
    m_log_table->setAutoFillBackground (false);
    m_log_table->setColumnCount (4);
    m_log_table->setCornerButtonEnabled (false);
    m_log_table->setEditTriggers (QAbstractItemView::SelectedClicked);
    m_log_table->setEnabled (true);
    m_log_table->setGridStyle (Qt::NoPen);
    m_log_table->setHorizontalScrollMode (QAbstractItemView::ScrollPerPixel);
    m_log_table->setLineWidth (0);
    m_log_table->setRowCount (0);
    m_log_table->setSelectionMode (QAbstractItemView::NoSelection);
    m_log_table->setShowGrid (false);
    m_log_table->setSortingEnabled (false);
    m_log_table->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
    m_log_table->setWordWrap (false);

    m_log_table->verticalHeader()->setVisible (false);

    m_log_table->setHorizontalHeaderLabels ( QStringList()
                                           << tr ("Time")
                                           << tr ("Source")
                                           << tr ("Location")
                                           << tr ("Message")
                                           );
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

    log_filter_dial->setValue (2);
    log_filter_dial->setSliderPosition (2);
    m_level_filter_selector->setCurrentIndex (2);

    connect ( m_level_filter_selector, SIGNAL (currentIndexChanged(int))
            , this, SLOT (levelFilterChanged(int))
            );

    connect ( log_filter_dial, SIGNAL (valueChanged(int))
            , m_level_filter_selector, SLOT (setCurrentIndex(int))
            );
    connect ( m_level_filter_selector, SIGNAL (currentIndexChanged(int))
            , log_filter_dial, SLOT (setValue(int))
            );

    m_drop_filtered->setCheckState (Qt::Checked);
    m_drop_filtered->setToolTip
      (tr ("Drop filtered events instead of keeping them"));


    QGroupBox* control_box (new QGroupBox (tr ("Control"), this));

    QPushButton* clear_log_button (new QPushButton (tr ("Clear"), this));
    clear_log_button->setToolTip (tr ("Clear all events"));
    connect (clear_log_button, SIGNAL (clicked()), this, SLOT (clearLogging()));

    QCheckBox* follow_logging_cb (new QCheckBox (tr ("follow"), this));
    follow_logging_cb->setChecked (true);
    follow_logging_cb->setToolTip ( tr ( "Follow the stream of log events and "
                                         "automatically scroll the view, drop "
                                         "events otherwise"
                                       )
                                  );
    connect ( follow_logging_cb, SIGNAL (toggled (bool))
            , this, SLOT (toggleFollowLogging (bool))
            );


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

    QGridLayout* logging_tab_layout (new QGridLayout (logging_tab));
    logging_tab_layout->addWidget (m_log_table, 0, 0);
    logging_tab_layout->addLayout (log_sidebar_layout, 0, 1);


    tab_widget->addTab (logging_tab, tr ("Logging"));
  }

  // --- execution monitor tab
  {
    QWidget* execution_tab (new QWidget (this));

    m_view->setAlignment (Qt::AlignRight | Qt::AlignTop);
    m_view->setDragMode (QGraphicsView::ScrollHandDrag);
    m_view->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
    m_view->setTransformationAnchor (QGraphicsView::AnchorViewCenter);
    m_view->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);

    m_component_view->setAlignment (Qt::AlignRight | Qt::AlignTop);
    m_component_view->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
    m_component_view->setRenderHint (QPainter::Antialiasing);
    m_component_view->setTransformationAnchor (QGraphicsView::NoAnchor);
    m_component_view->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);

    connect ( m_view->verticalScrollBar(), SIGNAL (valueChanged (int))
            , m_component_view->verticalScrollBar(), SLOT (setValue (int))
            );


    QSplitter* task_view_widget (new QSplitter (Qt::Horizontal, this));

    task_view_widget->addWidget (m_component_view);
    task_view_widget->addWidget (m_view);

    task_view_widget->setSizes (QList<int>() << 0 << 1);

    QGroupBox* legend_box (new QGroupBox (tr ("Legend"), this));

    QSignalMapper* legend_label_click (new QSignalMapper (this));
    connect ( legend_label_click, SIGNAL (mapped (QString))
            , this, SLOT (change_gantt_color (QString))
            );

    {
      QVBoxLayout* legend_box_layout (new QVBoxLayout (legend_box));

      QSettings settings;
      settings.beginGroup ("gantt");

      foreach ( const QString& state
              , QStringList()
              << "created" << "started" << "finished" << "failed" << "cancelled"
              )
      {
        int r, g, b;
        settings.value (state).value<QColor>().getRgb (&r, &g, &b);

        QPushButton* label (new QPushButton (state, this));
        label->setStyleSheet
          (QString ("background-color: rgb(%1, %2, %3)").arg (r).arg (g).arg (b));
        legend_box_layout->addWidget (label);

        connect (label, SIGNAL (clicked()), legend_label_click, SLOT (map()));
        legend_label_click->setMapping (label, state);
      }

      settings.endGroup();
    }

    QGroupBox* control_box (new QGroupBox (tr ("Control"), this));

    QPushButton* clear_log_button (new QPushButton (tr ("Clear"), this));
    clear_log_button->setToolTip (tr ("Clear all events"));
    connect ( clear_log_button, SIGNAL (clicked())
            , this, SLOT (clearActivityLog())
            );

    QCheckBox* follow_logging_cb (new QCheckBox (tr ("follow"), this));
    follow_logging_cb->setChecked (true);
    follow_logging_cb->setToolTip ( tr ( "Follow the stream of log events and "
                                         "automatically scroll the view"
                                       )
                                  );
    connect ( follow_logging_cb, SIGNAL (toggled (bool))
            , this, SLOT (toggleFollowTaskView (bool))
            );

    QSlider* zoom_slider (new QSlider (this));
    zoom_slider->setMinimum (1);
    zoom_slider->setMaximum (800);
    zoom_slider->setValue (100);
    zoom_slider->setOrientation (Qt::Horizontal);
    zoom_slider->setTickPosition (QSlider::TicksAbove);
    zoom_slider->setTickInterval (100);

    connect ( zoom_slider, SIGNAL (valueChanged (int))
            , this, SLOT (changeTaskViewZoom (int))
            );


    QVBoxLayout* control_box_layout (new QVBoxLayout (control_box));
    control_box_layout->addWidget (clear_log_button);
    control_box_layout->addWidget (follow_logging_cb);
    control_box_layout->addWidget (zoom_slider);
    zoom_slider->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

    QVBoxLayout* execution_sidebar_layout (new QVBoxLayout);
    execution_sidebar_layout->addWidget (legend_box);
    execution_sidebar_layout->addStretch();
    execution_sidebar_layout->addWidget (control_box);

    QGridLayout* execution_tab_layout (new QGridLayout (execution_tab));
    execution_tab_layout->addWidget (task_view_widget, 0, 0);
    execution_tab_layout->addLayout (execution_sidebar_layout, 0, 1);


    tab_widget->addTab (execution_tab, tr ("Execution Monitor"));
  }

  // --- meta

  setCentralWidget (tab_widget);
  setWindowTitle(tr ("SDPA Graphical Monitor"));
  tab_widget->setCurrentIndex (1);

  setMenuBar (new QMenuBar (this));

  QMenu* menuFile (menuBar()->addMenu (tr ("File")));
  menuFile->addAction (tr ("Save"), this, SLOT (save()), QKeySequence::Save);
  menuFile->addSeparator();
  menuFile->addAction (tr ("Exit"), this, SLOT (close()), QKeySequence::Quit);

  connect (&m_timer, SIGNAL(timeout()), this, SLOT(advance()));

  static const int updates_per_second (30);
  m_timer.start (1000 / updates_per_second);
}

MonitorWindow::~MonitorWindow()
{
  m_io_service.stop();
  m_io_thread.join();
}

void MonitorWindow::change_gantt_color (const QString& state)
{
  QSettings settings;

  QColor new_color
    ( QColorDialog::getColor ( settings.value ("gantt/" + state).value<QColor>()
                             , this
                             , tr ("Select new color for state %1").arg (state)
                             )
    );

  if (new_color.isValid())
  {
    settings.setValue ("gantt/" + state, new_color);

    int r, g, b;
    new_color.getRgb (&r, &g, &b);

    //! \todo This is _really_ ugly code. Any way to solve this more nicely?
    qobject_cast<QWidget*>
      (qobject_cast<QSignalMapper*> (sender())->mapping (state))->setStyleSheet
      (QString ("background-color: rgb(%1, %2, %3)").arg (r).arg (g).arg (b));

    {
      lock_type lock (m_task_view_mutex);
      foreach (QGraphicsItem* item, m_scene->items())
      {
        if (Task* task = qgraphicsitem_cast<Task*> (item))
        {
          task->reset_color();
        }
      }
    }
  }
}

void MonitorWindow::advance()
{
  scene_update_list_t updates;
  {
    lock_type lock (m_task_view_mutex);
    updates.swap (m_scene_updates);
  }

  BOOST_FOREACH (const scene_update_list_t::value_type& update, updates)
  {
    update.second->addItem (update.first);
  }

  QRectF scene_rect (m_scene->sceneRect());
  scene_rect.setWidth (scene_rect.width() + 1.0);
  scene_rect.setHeight (m_components.size() * 8);
  m_scene->setSceneRect (scene_rect);

  //! \todo do not call advance on all last tasks, but just advance
  // all 'active' elements.  i.e.  keep a list of currently active
  // elements
  BOOST_FOREACH (const std::string& component, m_components)
  {
    if (!m_tasks_list[component].empty())
    {
      m_tasks_list[component].back()->advance (scene_rect.width());
    }
  }

  if (m_follow_execution)
  {
    m_view->horizontalScrollBar()->setValue
      (m_view->horizontalScrollBar()->maximum());
  }
}

namespace
{
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
      return QColor (255,140,0);
    case fhg::log::LogLevel::ERROR:
      return QColor (255,0,0);
    case fhg::log::LogLevel::FATAL:
      return QColor (165,42,42);
    default:
      return QColor (0,0,0);
    }
  }

  template <typename T>
    void decode (const std::string& strMsg, T& t)
  {
    std::stringstream sstr(strMsg);
    boost::archive::text_iarchive ar(sstr);
    ar >> t;
  }
}

void MonitorWindow::append_exe (fhg::log::LogEvent const &evt)
{
  sdpa::daemon::NotificationEvent notification;
  try
  {
    decode(evt.message(), notification);
  }
  catch (const std::exception &ex)
  {
    return;
  }

  if (notification.activity_state() != sdpa::daemon::NotificationEvent::STATE_IGNORE)
  {
    we::mgmt::type::activity_t act (notification.activity());

    UpdateExecutionView(notification, act);
  }
  else
  {
    //    qDebug() << "activity " << notification.activity_id().c_str() << " failed!";
  }
}

void MonitorWindow::UpdateExecutionView
  ( const sdpa::daemon::NotificationEvent& event
  , const we::mgmt::type::activity_t& activity
  )
{
  static const qreal task_height (8.0);

  const std::string& component (event.component());
  std::string activity_name (event.activity_name());
  const std::string& activity_id (event.activity_id());

  try
  {
    const we::type::module_call_t mod_call
      (boost::get<we::type::module_call_t> (activity.transition().data()));
    activity_name = mod_call.module() + ":" + mod_call.function();
  }
  catch (boost::bad_get const &)
  {
    // do nothing, i.e. take the activity name as it was
  }

  const lock_type lock (m_task_struct_mutex);

  const std::vector<std::string>::iterator comp
    (std::find (m_components.begin(), m_components.end(), component));

  const qreal x_coord (m_scene->width());
  const qreal y_coord ( comp != m_components.end()
                      ? std::distance (m_components.begin(), comp) * task_height
                      : m_components.size() * task_height
                      );

  if (comp == m_components.end())
  {
    m_components.push_back (component);

    QGraphicsSimpleTextItem* label
      (new QGraphicsSimpleTextItem (QString::fromStdString (component)));

    QFont font (label->font());
    font.setPointSize (6);
    label->setFont (font);
    label->setPos (0, y_coord);

    m_scene_updates.push_back (std::make_pair (label, m_component_scene));
  }

  if ( m_tasks_grid[component].find (activity_id)
     == m_tasks_grid[component].end()
     )
  {
    Task* task ( new Task ( QString::fromStdString (component)
                          , QString::fromStdString (activity_name)
                          , QString::fromStdString (activity_id)
                          )
               );
    task->setPos (x_coord, y_coord);
    m_tasks_grid[component][activity_id] = task;

    // new task, make sure to close previous task -> asume finished
    if (!m_tasks_list[component].empty())
    {
      m_tasks_list[component].back()->update_task_state
        (sdpa::daemon::NotificationEvent::STATE_FINISHED);
    }
    m_tasks_list[component].push_back (task);

    lock_type lock (m_task_view_mutex);
    m_scene_updates.push_back (std::make_pair (task, m_scene));
  }

  m_tasks_grid[component][activity_id]->update_task_state
    (event.activity_state());
}

void MonitorWindow::clearActivityLog()
{
  lock_type struct_lock (m_task_struct_mutex);

  delete m_scene;
  delete m_component_scene;
  m_scene = new QGraphicsScene (this);
  m_component_scene = new QGraphicsScene (this);

  m_view->setScene (m_scene);
  m_component_view->setScene (m_component_scene);

  m_tasks_grid.clear();
  m_tasks_list.clear();
  m_components.clear();

  {
    lock_type update_lock (m_task_view_mutex);
    while (!m_scene_updates.empty())
    {
      delete m_scene_updates.front().first;
      m_scene_updates.pop_front();
    }
  }
}

static const int TABLE_COL_TIME = 0;
static const int TABLE_COL_SOURCE = 1;
static const int TABLE_COL_LOCATION = 2;
static const int TABLE_COL_MESSAGE = 3;

void MonitorWindow::append_log (fhg::log::LogEvent const &evt)
{
  if ( evt.severity() < m_level_filter_selector->currentIndex ()
     && m_drop_filtered->isChecked()
     )
  {
    return;
  }

  // TODO:
  //    store in internal list -> data
  //    update view (different views)
  //    respect filtering etc.
  //    maximum number of events (circular buffer like)

  if (m_log_events.size () > (1 << 15))
  {
    clearLogging ();
  }

  m_log_events.push_back (evt);

  int row (m_log_table->rowCount ());
  m_log_table->setRowCount (row+1);

  QColor bg(severityToColor(evt.severity()));
  QBrush fg(severityToColor(evt.severity()));

  {
    char buf[128]; memset (buf, 0, sizeof(buf));
    time_t tm = (evt.tstamp());
    ctime_r (&tm, buf);

    QTableWidgetItem *i(new QTableWidgetItem (buf));
    i->setForeground(fg);
    m_log_table->setItem(row, TABLE_COL_TIME, i);
  }

  {
    std::ostringstream sstr;
    sstr << evt.pid() << "@" << evt.logged_on();
    std::string logged_on;
    QTableWidgetItem *i(new QTableWidgetItem (sstr.str().c_str()));
    i->setForeground(fg);
    m_log_table->setItem(row, TABLE_COL_SOURCE, i);
  }

  {
    QTableWidgetItem *i
        (new QTableWidgetItem (( evt.file ()
                               + ":"
                               + boost::lexical_cast<std::string>(evt.line())).c_str()
                               )
        );
    i->setForeground (fg);
    m_log_table->setItem (row, TABLE_COL_LOCATION, i);
  }
  {
    QTableWidgetItem *i
        (new QTableWidgetItem (evt.message().c_str()));
    i->setForeground (fg);
    m_log_table->setItem (row, TABLE_COL_MESSAGE, i);
  }
  if (m_follow_logging)
    m_log_table->scrollToBottom ();
  m_log_table->resizeRowToContents (row);

  if (evt.severity() < m_level_filter_selector->currentIndex())
    m_log_table->setRowHidden (row, true);
  else
    m_log_table->setRowHidden (row, false);
}

namespace
{
  enum
  {
    EXTERNAL_EVENT_LOGGING = QEvent::User + 1,
  };

  class fhglog_event : public QEvent
  {
  public:
    fhglog_event (const fhg::log::LogEvent& e)
      : QEvent ((QEvent::Type)EXTERNAL_EVENT_LOGGING)
      , log_event (e)
    { }

    fhg::log::LogEvent log_event;
  };
}

void MonitorWindow::handle_external_event (const fhg::log::LogEvent & evt)
{
  QApplication::postEvent (this, new fhglog_event (evt));
}

bool MonitorWindow::event (QEvent* event)
{
  if (event->type() == EXTERNAL_EVENT_LOGGING)
  {
    event->accept();
    append_log (static_cast<fhglog_event*> (event)->log_event);
    return true;
  }

  return QWidget::event (event);
}

void MonitorWindow::clearLogging ()
{
  m_log_events.clear();
  m_log_table->clearContents();
  m_log_table->setRowCount (0);
}

void MonitorWindow::toggleFollowLogging (bool follow)
{
  m_follow_logging = follow;
}

void MonitorWindow::toggleFollowTaskView (bool follow)
{
  m_follow_execution = follow;
}

void MonitorWindow::levelFilterChanged (int lvl)
{
  for (size_t i = 0; i < m_log_events.size (); ++i)
    if (m_log_events[i].severity() < m_level_filter_selector->currentIndex())
      m_log_table->setRowHidden (i, true);
    else
      m_log_table->setRowHidden (i, false);
}

void MonitorWindow::changeTaskViewZoom (int to)
{
  qreal target = (to / 100.0);
  qreal factor = target / m_current_scale;

  m_view->scale (factor, factor);
  m_component_view->scale (factor, factor);

  m_current_scale = target;
}

void MonitorWindow::save ()
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
