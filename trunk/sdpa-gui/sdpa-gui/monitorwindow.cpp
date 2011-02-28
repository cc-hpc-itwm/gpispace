#include "monitorwindow.hpp"
#include "logeventwrapper.hpp"
#include "ui_monitorwindow.h"
#include "windowappender.hpp"
#include <QHeaderView>
#include <QApplication>

#include <boost/lexical_cast.hpp>

MonitorWindow::MonitorWindow( unsigned short exe_port
                            , unsigned short log_port
                            , QWidget *parent
                            ) :
    QMainWindow(parent),
    ui(new Ui::MonitorWindow),
    m_follow_logging (true)
{
    ui->setupUi(this);
    ui->m_log_table->horizontalHeader ()->setVisible (true);
    ui->m_log_table->
        horizontalHeaderItem (2)->setTextAlignment (Qt::AlignLeft);
    ui->m_log_table->
        setSelectionMode(QAbstractItemView::NoSelection);

    m_exe_server = logserver_t
        (new fhg::log::remote::LogServer
         ( fhg::log::Appender::ptr_t(new WindowAppender(this, 1001))
         , m_io_service, exe_port));
    m_log_server = logserver_t
        (new fhg::log::remote::LogServer
         ( fhg::log::Appender::ptr_t(new WindowAppender(this, 1002))
         , m_io_service, log_port));
    m_io_thread = thread_t
        (new boost::thread
         (boost::bind
          (&boost::asio::io_service::run, &m_io_service)));
}

MonitorWindow::~MonitorWindow()
{
  m_io_service.stop();
  m_io_thread->join ();
  m_io_thread.reset ();
  m_log_server.reset ();
  delete ui;
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

void
MonitorWindow::append_exe (fhg::log::LogEvent const &evt)
{
  // map source to column
  //    maybe create new column
  // parallel -> if !exist (finished in current row)
  //    add item to current row
  // else
  //    add new row
}

void
MonitorWindow::append_log (fhg::log::LogEvent const &evt)
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

void
MonitorWindow::clearLogging ()
{
  m_log_events.clear ();
  ui->m_log_table->clearContents ();
  ui->m_log_table->setRowCount (0);
}

void
MonitorWindow::toggleFollowLogging (bool follow)
{
  m_follow_logging = follow;
}

void
MonitorWindow::levelFilterChanged (int lvl)
{
  for (int i = 0; i < m_log_events.size (); ++i)
  {
    if (m_log_events[i].severity() < ui->m_level_filter_selector->currentIndex())
    {
      ui->m_log_table->setRowHidden (i, true);
    }
    else
    {
      ui->m_log_table->setRowHidden (i, false);
    }
  }
}
