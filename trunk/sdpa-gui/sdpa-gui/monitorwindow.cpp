#include "monitorwindow.hpp"
#include "logeventwrapper.hpp"
#include "ui_monitorwindow.h"
#include <QHeaderView>
#include <QApplication>

#include <boost/lexical_cast.hpp>

namespace
{
  class WindowAppender : public fhg::log::Appender
  {
  public:
    explicit
    WindowAppender (MonitorWindow *win)
      : fhg::log::Appender ("gui")
      , m_win (win)
    {}

    void append (fhg::log::LogEvent const &evt)
    {
      QApplication::postEvent (m_win, new LogEventWrapper(evt));
    }

    void flush()
    {}
  private:
    MonitorWindow *m_win;
  };
}

MonitorWindow::MonitorWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MonitorWindow)
{
    ui->setupUi(this);
    ui->m_log_table->horizontalHeader ()->setVisible (true);

    m_log_server = logserver_t
        (new fhg::log::remote::LogServer
         ( fhg::log::Appender::ptr_t(new WindowAppender(this))
         , m_io_service, 2439));
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

void
MonitorWindow::append (fhg::log::LogEvent const &evt)
{
  // todo:
  //    store in internal list -> data
  //    update view (different views)
  //    respect filtering etc.
  int row (ui->m_log_table->rowCount ());
  ui->m_log_table->setRowCount (row+1);
  ui->m_log_table->setItem
      (row, 0, new QTableWidgetItem (QString(evt.severity().str().c_str())));
  ui->m_log_table->setItem
      (row, 1, new QTableWidgetItem
       ( ( evt.file ()
         + ":"
         + boost::lexical_cast<std::string>(evt.line())
         ).c_str()
        )
       );
  ui->m_log_table->setItem
      (row, 2, new QTableWidgetItem(evt.message().c_str()));
}

bool
MonitorWindow::event (QEvent *e)
{
  if (e->type() == 1001)
  {
    e->accept ();
    LogEventWrapper *logevt = (LogEventWrapper*)(e);
    append (logevt->log_event);
    return true;
  }
  else
  {
   return QWidget::event(e);
  }
}
