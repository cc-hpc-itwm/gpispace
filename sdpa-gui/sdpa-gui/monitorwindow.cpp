#include "monitorwindow.hpp"
#include "logeventwrapper.hpp"
#include "ui_monitorwindow.h"
#include "windowappender.hpp"
#include "task.h"
#include <QHeaderView>
#include <QApplication>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QSplitter>
#include <QMessageBox>
#include <QScrollBar>
#include <QFileDialog>

#include <boost/lexical_cast.hpp>
#include <list>
#include <sstream>
#include <cmath>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/serialization/access.hpp>
#include <boost/bind.hpp>

#include <we/util/codec.hpp>
#include <we/loader/putget.hpp>
#include <we/type/token.hpp>

#include <sdpa-gui/taskview/taskscene.hpp>
#include <sdpa-gui/taskview/taskview.hpp>

#include "portfolioevent.hpp"

using namespace std;
using namespace boost;

static const int EXTERNAL_EVENT_EXECUTION = 1001;
static const int EXTERNAL_EVENT_LOGGING = 1002;

MonitorWindow::MonitorWindow( unsigned short exe_port
                            , unsigned short log_port
                            , QWidget *parent
                            )
  : QMainWindow(parent)
  , ui(new Ui::MonitorWindow)
  , m_follow_logging (true)
  , m_follow_execution (true)
  , m_portfolio_(new Portfolio(ui))
  , m_current_scale (1.0)
{
    ui->setupUi(this);
    ui->m_log_table->verticalHeader ()->setVisible (false);
    ui->m_log_table->horizontalHeader ()->setVisible (true);
    ui->m_log_table->horizontalHeader ()->setStretchLastSection(true);
    ui->m_log_table->horizontalHeaderItem (2)->setTextAlignment (Qt::AlignLeft);
    ui->m_log_table->setSelectionMode(QAbstractItemView::NoSelection);
    ui->m_drop_filtered->setCheckState(Qt::Checked);
    m_portfolio_->Init();

    m_scene = new fhg::taskview::TaskScene (this);
    m_view = new fhg::taskview::TaskView(m_scene);

    m_component_scene = new QGraphicsScene();
    m_component_view = new QGraphicsView();

    //    m_scene->setSceneRect(0,0,0,0);
    //    m_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    //    m_view->setScene (m_scene);
    //    m_view->setAttribute (Qt::WA_AlwaysShowToolTips);

    m_component_view->setScene(m_component_scene);
    {
      QSplitter *splitter = new QSplitter (Qt::Horizontal);
      QHBoxLayout *l = new QHBoxLayout;
      l->setSpacing(0);
      l->setContentsMargins(0,0,0,0);
      /*
      m_component_view->setSizePolicy( QSizePolicy( QSizePolicy::Minimum
                                                  , QSizePolicy::Minimum
                                                  )
                                     );
      m_view->setSizePolicy(QSizePolicy( QSizePolicy::Maximum
                                      , QSizePolicy::Maximum)
                          );
      m_view->setMinimumWidth(300);
      */
      splitter->addWidget(m_component_view);
      splitter->addWidget(m_view);
      splitter->setLayout(l);

      QList<int> sizes;
      sizes << 0 << 1;
      splitter->setSizes(sizes);

      l = new QHBoxLayout;
      l->addWidget(splitter);
      ui->task_view_widget->setLayout(l);
    }

    m_component_view->setAlignment(Qt::AlignRight | Qt::AlignTop);
    m_component_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_component_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_component_view->setRenderHint(QPainter::Antialiasing);
    m_component_view->setTransformationAnchor(QGraphicsView::NoAnchor);

    //    m_view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);
    //    m_view->setAlignment(Qt::AlignRight | Qt::AlignTop);
    //    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //m_view->setRenderHint(QPainter::Antialiasing);

    //    m_view->setCacheMode(QGraphicsView::CacheNone);
    //m_view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    //m_view->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    //    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    //    m_view->setDragMode(QGraphicsView::ScrollHandDrag);

    m_exe_server = logserver_t
        (new fhg::log::remote::LogServer
            (fhg::log::Appender::ptr_t
                (new WindowAppender
                    (boost::bind
                        ( &MonitorWindow::append_exe
                        , this
                        , _1
                        )
                    )
                )
            , m_io_service, exe_port
            )
        );

    m_log_server = logserver_t
        (new fhg::log::remote::LogServer
            (fhg::log::Appender::ptr_t
                (new WindowAppender
                    (boost::bind
                        ( &MonitorWindow::handle_external_event
                        , this
                        , EXTERNAL_EVENT_LOGGING
                        , _1
                        )
                    )
                )
            , m_io_service, log_port
            )
        );

    m_io_thread = thread_t
        (new boost::thread
         (boost::bind
          (&boost::asio::io_service::run, &m_io_service)));

    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(advance()));
    QObject::connect( m_view->verticalScrollBar(), SIGNAL(valueChanged(int))
                    , m_component_view->verticalScrollBar(), SLOT(setValue(int))
                    );

    m_timer.start((int)(std::floor (1000 / 26. + 0.5)));
    m_portfolio_->InitTable();
}

MonitorWindow::~MonitorWindow()
{
  m_io_service.stop();
  m_io_thread->join ();
  m_io_thread.reset ();
  m_log_server.reset ();
  delete ui;
}

void MonitorWindow::advance()
{
  scene_update_list_t updates;
  {
    lock_type lock (m_task_view_mutex);
    updates.swap(m_scene_updates);
  }
  for ( scene_update_list_t::iterator update (updates.begin())
      ; update != updates.end()
      ; ++update
      )
  {
    update->second->addItem (update->first);
  }

  QRectF scene_rect = m_scene->sceneRect();
  scene_rect.setWidth(scene_rect.width() + 1.0);
  scene_rect.setHeight(m_components.size() * 8);
  m_scene->setSceneRect(scene_rect);
  // TODO: do not call scene::advance but just advance all 'active' elements
  //       i.e. keep a list of currently active elements
  m_scene->advance();

  if (m_follow_execution)
    m_view->horizontalScrollBar()->setValue(m_view->horizontalScrollBar()->maximum());
}

static QColor severityToColor (const fhg::log::LogLevel lvl)
{
  switch (lvl.lvl ())
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

void MonitorWindow::UpdatePortfolioView( sdpa::daemon::NotificationEvent const & evt
                                       , we::mgmt::type::activity_t const & act
                                       )
{
  if (evt.activity_state() != sdpa::daemon::NotificationEvent::STATE_FINISHED)
  {
    return;
  }

  try
  {
    we::type::module_call_t mod_call
      (boost::get<we::type::module_call_t>(act.transition().data()));
    if (mod_call.module() == "asian")
    {
      int val = ui->m_progressBar->value()+1;
      ui->m_progressBar->setValue(val);
    }

    if (mod_call.function() == "done")
    {
      we::mgmt::type::activity_t::output_t output (act.output());

      for ( we::mgmt::type::activity_t::output_t::const_iterator it(output.begin())
          ; it != output.end()
          ; ++it
          )
      {
        using namespace we::loader;
        token::type token (it->first);

        long rowId (get<long>(token.value, "rowID"));
        double pv (get<double>(token.value, "pv"));
        double stddev(get<double>(token.value, "stddev"));
        double Delta(get<double>(token.value, "Delta"));
        double Gamma(get<double>(token.value, "Gamma"));
        double Vega(get<double>(token.value, "Vega"));

        simulation_result_t sim_res(rowId, pv, stddev, Delta, Gamma, Vega);
        m_portfolio_->ShowResult(sim_res);
      }
    }
  }
  catch (std::exception const &ex)
  {
    // ignore non module calls...
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
    we::mgmt::type::activity_t act;
    try
    {
      we::util::codec::decode(notification.activity(), act);
    }
    catch (std::exception const &ex)
    {
      return;
    }

    UpdateExecutionView(notification, act);
    QApplication::postEvent (this, new PortFolioEvent(1001, notification, act));
  }
  else
  {
    //    qDebug() << "activity " << notification.activity_id().c_str() << " failed!";
  }
}

void MonitorWindow::UpdateExecutionView( sdpa::daemon::NotificationEvent const & evt
                                       , we::mgmt::type::activity_t const & act
                                       )
{
  static const int task_height (8);

  std::string const & component (evt.component());
  std::string         activity_name (evt.activity_name());
  std::string const & activity_id (evt.activity_id());
  sdpa::daemon::NotificationEvent::state_t activity_state (evt.activity_state());

  try
  {
    we::type::module_call_t mod_call
      (boost::get<we::type::module_call_t>(act.transition().data()));
    activity_name = mod_call.module() + ":" + mod_call.function();
  }
  catch (boost::bad_get const &)
  {
    // do nothing, i.e. take the activity name as it was
  }

  lock_type lock(m_task_struct_mutex);

  // look for component
  qreal       y_coord = -1;
  const qreal x_coord = m_scene->width();

  for ( std::vector<std::string>::iterator comp = m_components.begin()
      ; comp != m_components.end()
      ; ++comp
      )
  {
    if (*comp == component)
    {
      y_coord = (comp - m_components.begin()) * task_height;
      break;
    }
  }

  if (y_coord < 0)
  {
    y_coord = m_components.size() * task_height;
    m_components.push_back (component);

    QGraphicsSimpleTextItem *c_label = new QGraphicsSimpleTextItem(QString(component.c_str()));
    QFont font = c_label->font();
    font.setPointSize(6);
    c_label->setFont (font);
    c_label->setPos(0, y_coord);

    m_scene_updates.push_back (std::make_pair(c_label, m_component_scene));
  }

  // look for activity in activity-id map
  id_to_task_map_t & id_to_task = m_tasks_grid[component];
  id_to_task_map_t::iterator task_it = id_to_task.find(activity_id);

  Task *task = 0;

  if (task_it == id_to_task.end())
  {
    task = new Task(component.c_str(), activity_name.c_str(), activity_id.c_str());
    task->setPos(x_coord, y_coord);
    id_to_task[activity_id] = task;

    // new task, make sure to close previous task -> asume finished
    if (! m_tasks_list[component].empty())
    {
      m_tasks_list[component].back()->update_task_state(sdpa::daemon::NotificationEvent::STATE_FINISHED);
    }
    m_tasks_list[component].push_back(task);

    lock_type lock (m_task_view_mutex);
    m_scene_updates.push_back (std::make_pair(task, m_scene));
  }
  else
  {
    task = task_it->second;
  }
  task->update_task_state(activity_state);
}

void MonitorWindow::clearActivityLog()
{
  lock_type struct_lock(m_task_struct_mutex);

  delete m_scene; delete m_component_scene;
  m_scene = new fhg::taskview::TaskScene(this);
  m_component_scene = new QGraphicsScene();

  m_view->setScene(m_scene);
  m_component_view->setScene(m_component_scene);

  m_tasks_grid.clear();
  m_tasks_list.clear();
  m_components.clear();

  {
    lock_type update_lock(m_task_view_mutex);
    while (!m_scene_updates.empty())
    {
      QGraphicsItem *t = m_scene_updates.front().first;
      delete t;
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
  if (not m_follow_logging)
  {
    return;
  }

  if ( evt.severity() < ui->m_level_filter_selector->currentIndex ()
     && ui->m_drop_filtered->isChecked()
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

  int row (ui->m_log_table->rowCount ());
  ui->m_log_table->setRowCount (row+1);

  QColor bg(severityToColor(evt.severity()));
  QBrush fg(severityToColor(evt.severity()));

  {
    char buf[128]; memset (buf, 0, sizeof(buf));
    time_t tm = (evt.tstamp());
    ctime_r (&tm, buf);

    QTableWidgetItem *i(new QTableWidgetItem (buf));
    i->setForeground(fg);
    ui->m_log_table->setItem(row, TABLE_COL_TIME, i);
  }

  {
    std::ostringstream sstr;
    sstr << evt.pid() << "@" << evt.logged_on();
    std::string logged_on;
    QTableWidgetItem *i(new QTableWidgetItem (sstr.str().c_str()));
    i->setForeground(fg);
    ui->m_log_table->setItem(row, TABLE_COL_SOURCE, i);
  }

  {
    QTableWidgetItem *i
        (new QTableWidgetItem (( evt.file ()
                               + ":"
                               + boost::lexical_cast<std::string>(evt.line())).c_str()
                               )
        );
    i->setForeground (fg);
    ui->m_log_table->setItem (row, TABLE_COL_LOCATION, i);
  }
  {
    QTableWidgetItem *i
        (new QTableWidgetItem (evt.message().c_str()));
    i->setForeground (fg);
    ui->m_log_table->setItem (row, TABLE_COL_MESSAGE, i);
  }
  if (m_follow_logging)
    ui->m_log_table->scrollToBottom ();
  ui->m_log_table->resizeRowToContents (row);

  if (evt.severity() < ui->m_level_filter_selector->currentIndex())
    ui->m_log_table->setRowHidden (row, true);
  else
    ui->m_log_table->setRowHidden (row, false);
}

void
MonitorWindow::handle_external_event ( int type
                                     , const fhg::log::LogEvent & evt
                                     )
{
  QApplication::postEvent (this, new LogEventWrapper(type, evt));
}

bool
MonitorWindow::event (QEvent *e)
{
  if (e->type() == 1001)
  {
    e->accept ();
    PortFolioEvent *evt = (PortFolioEvent*)(e);
    UpdatePortfolioView(evt->notification, evt->activity);
    return true;
  }
  if (e->type() == 1002)
  {
    e->accept ();
    LogEventWrapper *logevt = (LogEventWrapper*)(e);
    append_log (logevt->log_event);
    return true;
  }
  else
  {
   return QWidget::event(e);
  }
}

void MonitorWindow::clearLogging ()
{
        m_log_events.clear ();
        ui->m_log_table->clearContents ();
        ui->m_log_table->setRowCount (0);
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
    if (m_log_events[i].severity() < ui->m_level_filter_selector->currentIndex())
      ui->m_log_table->setRowHidden (i, true);
    else
      ui->m_log_table->setRowHidden (i, false);
}

void MonitorWindow::changeTaskViewZoom(int to)
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
