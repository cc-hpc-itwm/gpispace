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

void MonitorWindow::decode (const std::string& strMsg, sdpa::daemon::ApplicationGuiEvent& evtNotification)
{
	std::stringstream sstr(strMsg);
	boost::archive::text_iarchive ar(sstr);
	ar >> evtNotification;
}

void MonitorWindow::UpdatePortfolioView(fhg::log::LogEvent const &evt)
{
	sdpa::daemon::ApplicationGuiEvent evtNotification;
	try
	{
		decode(evt.message(), evtNotification);
	}
	catch (const std::exception &ex)
	{
	  std::cerr << "ignoring invalid event!" << std::endl;
	  return;
	}

	QString qstrRow = QString("%1").arg(evtNotification.row());
	QString qstrCol = QString("%1").arg(evtNotification.col());

	// evtNotification.result() it's a simple string, not encoded!
	std::string result(evtNotification.result());

	QString qstrResult(result.c_str());

	qDebug()<<"*************************************";
	qDebug()<<"Received new logging event: row = "<<qstrRow<<", col = "<<qstrCol;
	qDebug()<<"The result is: "<<qstrResult;
	qDebug()<<"**************************************";

	double pv = 0.0, stddev = 0.0, Delta = 0.0, Gamma = 0.0, Vega = 0.0;
	int rowId = 0;

	char_separator<char> sep("[,]");
	tokenizer<char_separator<char> > tokens(result, sep);
	BOOST_FOREACH(string t, tokens)
	{
	   QString qstr(t.c_str());
	   //qDebug()<<"token: "<<qstr;

	   if(t.find("Delta") != std::string::npos )
	   {
		   QStringList list = qstr.split(":=");
		   qDebug()<<QString(list[0])<< " -> "<<list[1];
		   Delta = list[1].toDouble();
	   }

	   if(t.find("Gamma") != std::string::npos )
	   {
		   QStringList list = qstr.split(":=");
		   qDebug()<<QString(list[0])<< " -> "<<list[1];
		   Gamma = list[1].toDouble();
	   }

	   if(t.find("Vega") != std::string::npos)
	   {
		   QStringList list = qstr.split(":=");
		   qDebug()<<QString(list[0])<< " -> "<<list[1];
		   Vega = list[1].toDouble();
	   }

	   if(t.find("pv") != std::string::npos )
	   {
		   QStringList list = qstr.split(":=");
		   qDebug()<<QString(list[0])<< " -> "<<list[1];
		   pv = list[1].toDouble();
	   }

	   if(t.find("stddev") != std::string::npos )
	   {
		   QStringList list = qstr.split(":=");
		   qDebug()<<QString(list[0])<< " -> "<<list[1];
		   stddev = list[1].toDouble();
	   }

	   if(t.find("rowID") != std::string::npos )
	   {
		   QStringList list = qstr.split(":=");
		   qDebug()<<QString(list[0])<< " -> "<<list[1];
		   rowId = list[1].toInt();
	   }
	}

	simulation_result_t sim_res(rowId, pv, stddev, Delta, Gamma, Vega);
	m_portfolio_.ShowResult(sim_res);
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
