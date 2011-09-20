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

#include <boost/lexical_cast.hpp>
#include <list>
#include <sstream>
#include <cmath>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/serialization/access.hpp>
#include <boost/bind.hpp>

#include <we/we.hpp>
#include <we/util/codec.hpp>
#include <we/loader/putget.hpp>

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
    //  , m_portfolio_(ui)
  , m_current_scale (1.0)
{
    ui->setupUi(this);
    ui->m_log_table->horizontalHeader ()->setVisible (true);
    ui->m_log_table->horizontalHeaderItem (2)->setTextAlignment (Qt::AlignLeft);
    ui->m_log_table->setSelectionMode(QAbstractItemView::NoSelection);

    //    m_portfolio_.Init();

    m_scene.setSceneRect(0,0,0,0);
    m_scene.setItemIndexMethod(QGraphicsScene::NoIndex);
    m_view.setScene (&m_scene);
    m_view.setAttribute (Qt::WA_AlwaysShowToolTips);

    m_component_view.setScene(&m_component_scene);
    {
      QSplitter *splitter = new QSplitter (Qt::Horizontal);
      QHBoxLayout *l = new QHBoxLayout;
      l->setSpacing(0);
      l->setContentsMargins(0,0,0,0);
      m_component_view.setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
      m_view.setSizePolicy(QSizePolicy( QSizePolicy::Maximum
                                      , QSizePolicy::Maximum)
                          );
      //      m_component_view.setMinimumWidth(100);
      m_view.setMinimumWidth(300);
      splitter->addWidget(&m_component_view);
      splitter->addWidget(&m_view);
      //      ui->task_view_widget->setLayout(l);
      splitter->setCollapsible(0, false);
      splitter->setCollapsible(1, false);
      splitter->setLayout(l);

      l = new QHBoxLayout;
      l->addWidget(splitter);
      ui->task_view_widget->setLayout(l);
    }

    m_component_view.setAlignment(Qt::AlignRight | Qt::AlignTop);
    m_component_view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_component_view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    {
      for (int i = 0 ; i < 100; ++i)
      {
        QGraphicsSimpleTextItem *component = new QGraphicsSimpleTextItem(QString("component"));
        QFont font = component->font();
        font.setPointSize(6);
        component->setFont (font);
        m_component_scene.addItem(component);
        component->setPos(0, (i * 10));
      }
      QRectF sceneRect = m_component_scene.sceneRect();
      QSize  view_size = m_component_view.size();
      //      view_size.setWidth(sceneRect.width());
      //      m_component_view.resize (view_size);
      //      m_component_view.setMaximumWidth(view_size.width());
    }

    m_view.setAlignment(Qt::AlignRight | Qt::AlignTop);
    m_view.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_view.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    //m_view.setRenderHint(QPainter::Antialiasing);

    m_view.setCacheMode(QGraphicsView::CacheNone);
    m_view.setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    //m_view.setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    m_view.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_view.setDragMode(QGraphicsView::ScrollHandDrag);

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
    QObject::connect( m_view.verticalScrollBar(), SIGNAL(valueChanged(int))
                    , m_component_view.verticalScrollBar(), SLOT(setValue(int))
                    );

    m_timer.start(1000 / 26.);
    //    m_portfolio_.InitTable();
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
  static int counter = 0;

  QRectF old_scene_rect = m_scene.sceneRect();
  m_scene.advance();
  QRectF new_scene_rect = m_scene.sceneRect();

  if (old_scene_rect.width() < new_scene_rect.width())
  {
    if (m_follow_execution)
      m_view.horizontalScrollBar()->setValue(m_view.horizontalScrollBar()->maximum());
  }

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

  // switch update list with empty list
  // add all items to designated scene

  /*
  else if (++counter < 10)
  {
    const int num_new_tasks = qrand() % 1000;
    const int w = m_scene.width();
    for (int i = 0; i < num_new_tasks; ++i)
    {
      Task *task = new Task;
      m_scene.addItem(task);
      task->setPos(w, ( (i+counter)%num_new_tasks*10)); // get y coordinate from worker map
    }
  }
  */
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

/*
void MonitorWindow::UpdatePortfolioView( sdpa::daemon::NotificationEvent const & evt
                                       , we::activity_t const & act
                                       )
{
  if (evt.activity_state() != sdpa::daemon::NotificationEvent::STATE_FINISHED)
  {
    return;
  }

  we::activity_t::output_t output (act.output());

  qDebug() << evt.activity_name().c_str() << " produced " << output.size() << " token(s):";

  for ( we::activity_t::output_t::const_iterator it(output.begin())
      ; it != output.end()
      ; ++it
      )
  {
    using namespace we::loader;
    we::token_t token (it->first);

    qDebug() << "    " << boost::lexical_cast<std::string>(token).c_str();

    if (evt.activity_name () == "done")
    {
      long rowId (get<long>(token.value, "rowID"));
      double pv (get<double>(token.value, "pv"));
      double stddev(get<double>(token.value, "stddev"));
      double Delta(get<double>(token.value, "Delta"));
      double Gamma(get<double>(token.value, "Gamma"));
      double Vega(get<double>(token.value, "Vega"));

      simulation_result_t sim_res(rowId, pv, stddev, Delta, Gamma, Vega);
      m_portfolio_.ShowResult(sim_res);
    }
  }

  int val = ui->m_progressBar->value()+1;
  ui->m_progressBar->setValue(val);
}
*/

void MonitorWindow::append_exe (fhg::log::LogEvent const &evt)
{
  std::cerr << "BEEP!" << std::endl;

  sdpa::daemon::NotificationEvent notification;
  try
  {
    decode(evt.message(), notification);
  }
  catch (const std::exception &ex)
  {
    //    qDebug() << "ignoring invalid event!";
    return;
  }

  if (notification.activity_state() != sdpa::daemon::NotificationEvent::STATE_IGNORE)
  {
    we::activity_t act;
    try
    {
      we::util::text_codec::decode(notification.activity(), act);
    }
    catch (std::exception const &ex)
    {
      qDebug() << "could not parse activity: " << ex.what();
      return;
    }

    //    UpdatePortfolioView(notification, act);
    UpdateExecutionView(notification, act);
  }
  else
  {
    //    qDebug() << "activity " << notification.activity_id().c_str() << " failed!";
  }
}

void MonitorWindow::UpdateExecutionView( sdpa::daemon::NotificationEvent const & evt
                                       , we::activity_t const & act
                                       )
{
  std::string const & component (evt.component());
  std::string const & activity_name (evt.activity_name());
  std::string const & activity_id (evt.activity_id());
  sdpa::daemon::NotificationEvent::state_t activity_state (evt.activity_state());

  static int counter (0);

  const int w = m_scene.width();

  Task *task = new Task(component.c_str(), activity_name.c_str(), activity_id.c_str());
  task->setPos(w, counter*10); // get y coordinate from worker map
  ++counter;

  std::cerr << "added task: " << activity_name << std::endl;

  lock_type lock (m_task_view_mutex);
  m_scene_updates.push_back (std::make_pair(task, &m_scene));

  // look for activity in activity-id map
  //   if found -> ok, modify found task item
  //   else:
  //     look for component in component list
  //        if found -> ok, use found offset
  //        else     -> add component take last offset + delta
  //                    add graphics-item to list of new items + target scene
  //     add new task with found offset at the end of the scene
  //     make sure to 'close' last task on the same if there is one

  /*
  QTableWidgetItem *i(new QTableWidgetItem (evt.activity_name().c_str()));
  {
    std::ostringstream sstr;
    sstr << evt.activity_name() << std::endl;
    sstr << "Input: ";
    sstr << std::endl;
    for ( we::activity_t::output_t::const_iterator it(act.input().begin())
        ; it != act.input().end()
        ; ++it
        )
    {
      sstr
        << "   "
        << act.transition().get_port(it->second).name()
        << " = "
        << it->first
        << std::endl;
    }

    if (act.output().size())
    {
      sstr << std::endl;
      sstr << "Output: ";
      sstr << std::endl;
      for ( we::activity_t::output_t::const_iterator it(act.output().begin())
          ; it != act.output().end()
          ; ++it
          )
      {
      sstr
        << "   "
        << act.transition().get_port(it->second).name()
        << " = "
        << it->first
        << std::endl;
      }
    }
    i->setToolTip(sstr.str().c_str());
  }

  switch (evt.activity_state())
  {
  case sdpa::daemon::NotificationEvent::STATE_CREATED:
    i->setBackground(QBrush(QColor(128,128,128)));
    break;
  case sdpa::daemon::NotificationEvent::STATE_STARTED:
    i->setBackground(QBrush(QColor(255,255,0)));
    break;
  case sdpa::daemon::NotificationEvent::STATE_FINISHED:
    i->setBackground(QBrush(QColor(0,200,0)));
    if ((col+1) == ui->m_execution_log->columnCount())
    {
      m_current_col = ui->m_execution_log->columnCount();
    }
    break;
  case sdpa::daemon::NotificationEvent::STATE_FAILED:
    i->setBackground(QBrush(QColor(255,0,0)));
    if ((col+1) == ui->m_execution_log->columnCount())
    {
      m_current_col = ui->m_execution_log->columnCount();
    }
    break;
  case sdpa::daemon::NotificationEvent::STATE_CANCELLED:
    i->setBackground(QBrush(QColor(165,42,42)));
    if ((col+1) == ui->m_execution_log->columnCount())
    {
      m_current_col = ui->m_execution_log->columnCount();
    }
    break;
  default:
    return;
  }

  if (true)
    ui->m_execution_log->scrollToItem (i);
  */
}

void MonitorWindow::clearActivityLog()
{
  /*
  ui->m_execution_log->setRowCount (0);
  ui->m_execution_log->setColumnCount (0);
  ui->m_execution_log->clear ();

  m_node_to_row.clear ();
  m_activity_to_cell.clear ();
  m_current_col = 0;
  */
}

void MonitorWindow::append_log (fhg::log::LogEvent const &evt)
{

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
  m_log_events.push_back (evt);

  int row (ui->m_log_table->rowCount ());
  ui->m_log_table->setRowCount (row+1);

  QColor bg(severityToColor(evt.severity()));
  QBrush fg(severityToColor(evt.severity()));
  {
    QTableWidgetItem *i(new QTableWidgetItem (evt.logged_on ().c_str()));
    i->setForeground(fg);
    ui->m_log_table->setItem(row, 0, i);
  }
  {
    QTableWidgetItem *i
        (new QTableWidgetItem (( evt.file ()
                               + ":"
                               + boost::lexical_cast<std::string>(evt.line())).c_str()
                               )
        );
    i->setForeground (fg);
    ui->m_log_table->setItem (row, 1, i);
  }
  {
    QTableWidgetItem *i
        (new QTableWidgetItem (evt.message().c_str()));
    i->setForeground (fg);
    ui->m_log_table->setItem (row, 2, i);
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
    LogEventWrapper *logevt = (LogEventWrapper*)(e);
    append_exe (logevt->log_event);
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

  m_view.scale (factor, factor);
  m_component_view.scale (factor, factor);

  m_current_scale = target;
}
