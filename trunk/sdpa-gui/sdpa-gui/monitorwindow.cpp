#include "monitorwindow.hpp"
#include "logeventwrapper.hpp"
#include "ui_monitorwindow.h"
#include "windowappender.hpp"
#include <QHeaderView>
#include <QApplication>
#include <QDebug>

#include <boost/lexical_cast.hpp>
#include <list>
#include <sstream>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
//#include <sdpa/engine/IWorkflowEngine.hpp>
#include <boost/serialization/access.hpp>

#include <we/we.hpp>
#include <we/mgmt/layer.hpp>
#include <we/util/codec.hpp>
#include <we/loader/putget.hpp>

using namespace std;
using namespace boost;

MonitorWindow::MonitorWindow( unsigned short exe_port
                            , unsigned short log_port
                            , QWidget *parent
                            ) :
    QMainWindow(parent),
    ui(new Ui::MonitorWindow),
    m_portfolio_(ui),
    m_follow_logging (true)
{
    ui->setupUi(this);
    ui->m_log_table->horizontalHeader ()->setVisible (true);
    ui->m_log_table->horizontalHeaderItem (2)->setTextAlignment (Qt::AlignLeft);
    ui->m_log_table->setSelectionMode(QAbstractItemView::NoSelection);

    m_portfolio_.Init();

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

    m_portfolio_.InitTable();
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

template <typename T>
void decode (const std::string& strMsg, T& t)
{
	std::stringstream sstr(strMsg);
	boost::archive::text_iarchive ar(sstr);
	ar >> t;
}

void MonitorWindow::UpdatePortfolioView(fhg::log::LogEvent const &evt)
{
	sdpa::daemon::NotificationEvent evtNotification;
	try
	{
		decode(evt.message(), evtNotification);
	}
	catch (const std::exception &ex)
	{
	  qDebug() << "ignoring invalid event!";
	  return;
	}

	if (evtNotification.activity_state() != sdpa::daemon::NotificationEvent::STATE_FINISHED)
	{
		return;
	}

	we::activity_t act;

	try
	{
		we::util::text_codec::decode(evtNotification.activity_result(), act);
	}
	catch (std::exception const &ex)
	{
		qDebug() << "could not parse activity: " << ex.what();
		return;
	}

	we::activity_t::output_t output (act.output());

	qDebug() << evtNotification.activity_name ().c_str() << " produced " << output.size() << " token(s):";

	for ( we::activity_t::output_t::const_iterator it(output.begin())
        ; it != output.end()
        ; ++it
        )
	{
		using namespace we::loader;
		we::token_t token (it->first);

		qDebug() << "    " << boost::lexical_cast<std::string>(token).c_str();

		if (evtNotification.activity_name () == "done")
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

void MonitorWindow::append_exe (fhg::log::LogEvent const &evt)
{
	UpdatePortfolioView(evt);
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

void MonitorWindow::levelFilterChanged (int lvl)
{
	for (int i = 0; i < m_log_events.size (); ++i)
		if (m_log_events[i].severity() < ui->m_level_filter_selector->currentIndex())
			ui->m_log_table->setRowHidden (i, true);
		else
			ui->m_log_table->setRowHidden (i, false);
}
