//main.cpp

#include <list>
#include <iostream>
#include <pthread.h>
#include <boost/asio.hpp>

#include <fhglog/remote/LogServer.hpp>
#include <fhglog/Appender.hpp>
#include <fhglog/fhglog.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/thread.hpp>

#include "sdpaWnd.h"
#include "CostumerEvent.h"
#include <QApplication>

using namespace std;

void* service_thread_entry(void *arg)
{
  boost::asio::io_service *io(reinterpret_cast<boost::asio::io_service*>(arg));
  io->run();
  return NULL;
}

class GuiAppender : public fhg::log::Appender
{
  public:
    explicit
    GuiAppender(SdpaWnd *gui)
      : fhg::log::Appender("gui")
      , gui_(gui)
    {
	// start the update thread
	update_thread_ = boost::thread(boost::bind(&GuiAppender::update_thread, this));
    }

    virtual ~GuiAppender() throw ()
	{
	    update_thread_.interrupt();
	    update_thread_.join();
	}


  void append(const fhg::log::LogEvent &evt)
  {
      typedef std::map<std::string, int> id_map_t;
      static int id_counter(0);
      static id_map_t id_map;

    std::string sender(evt.logged_on());

    sdpa::daemon::NotificationEvent n_event;
    try
    {
      std::stringstream sstr (evt.message());
      boost::archive::text_iarchive ar(sstr);
      ar >> n_event;
    }
    catch (const std::exception &ex)
    {
      std::cerr << "ignoring invalid event!" << std::endl;
      return;
    }

    const std::string act_id = n_event.activity_id();
    std::string act_nm = n_event.activity_name();
    act_nm = act_nm.substr(act_nm.find_first_of(".")+1);

    const sdpa::daemon::NotificationEvent::state_t act_st = n_event.activity_state();

    id_map_t::iterator numeric_act_id = id_map.find(act_id);
    if (numeric_act_id == id_map.end())
    {
	std::pair<id_map_t::iterator, bool> insert_result =
	    id_map.insert(std::make_pair(act_id, id_counter));
	numeric_act_id = insert_result.first;
	id_counter++;
    }

    WndUpdateParameter param;
    param.id = numeric_act_id->second;
    memset( param.name, 0, WndUpdateParameter::max_len );
    memset( param.info, 0, WndUpdateParameter::max_len );
    strncpy (param.name, act_nm.c_str(), WndUpdateParameter::max_len);
    switch (act_st)
    {
	case sdpa::daemon::NotificationEvent::STATE_CREATED:
	    param.state = STATE_CREATE;
	    strncpy(param.info, sender.c_str(), WndUpdateParameter::max_len);
	    break;
	case sdpa::daemon::NotificationEvent::STATE_STARTED:
	    param.state = STATE_RUN;
	    strncpy(param.info, sender.c_str(), WndUpdateParameter::max_len);
	    break;
	case sdpa::daemon::NotificationEvent::STATE_FINISHED:
	    param.state = STATE_OK;
	    strncpy(param.info, "", WndUpdateParameter::max_len);
	    id_map.erase(act_id);
	    break;
	case sdpa::daemon::NotificationEvent::STATE_FAILED:
	    id_map.erase(act_id);
	    strncpy(param.info, "", WndUpdateParameter::max_len);
	    id_map.erase(act_id);
	    param.state = STATE_FAILED;
	    break;
	case sdpa::daemon::NotificationEvent::STATE_CANCELLED:
	    id_map.erase(act_id);
	    strncpy(param.info, "cancel", WndUpdateParameter::max_len);
	    id_map.erase(act_id);
	    param.state = STATE_FAILED;
	    break;
	default:
	    return;
    }

    boost::unique_lock<boost::recursive_mutex> lock(mtx_);
    event_t *dce = new event_t( (QEvent::Type)1001, param );
    event_queue_.push_back(dce);
    // we do not notify the update_thread, since he does a timed wait
  }

  void flush () {}

  private:
    void update_thread()
    {
	for (;;)
	{
	    try
	    {
		event_t *evt(NULL);
		{
		    boost::unique_lock<boost::recursive_mutex> lock(mtx_);
		    boost::system_time timeout(boost::get_system_time() + boost::posix_time::milliseconds(5));
		    event_available_.timed_wait(lock, timeout);
		    if (! event_queue_.empty())
		    {
			  evt = event_queue_.front(); event_queue_.pop_front();
		    }
		}
		if (evt)
		{
		    QCoreApplication::postEvent( gui_, evt );
		}
	    }
	    catch (const boost::thread_interrupted &irq)
	    {
		std::cout << "interrupted..." << std::endl;
		break;
	    }
	}
    }

    SdpaWnd *gui_;
    boost::recursive_mutex mtx_;
    boost::condition_variable_any event_available_;
    boost::thread update_thread_;
    typedef DataCostumerEvent<WndUpdateParameter> event_t;
    typedef std::list<event_t*> event_list_t;
    event_list_t event_queue_;
};

int main(int argc, char *argv[])
{
  fhg::log::Configurator::configure();

  if (argc <= 1)
  {
      std::cerr << "usage: " << argv[0] << " port" << std::endl;
      std::cerr << "\thint: default SDPA-port is 9001" << std::endl;
      return (EXIT_FAILURE);
  }
  int port (atoi(argv[1]));

  pthread_t service_thread;

  QApplication qa(argc, argv);
  SdpaWnd *wnd = new SdpaWnd();
  boost::asio::io_service io_service;
  fhg::log::Appender::ptr_t gui_appender(new GuiAppender(wnd));
  fhg::log::shared_ptr<fhg::log::remote::LogServer> logserver (new fhg::log::remote::LogServer(gui_appender, io_service, port));
  pthread_create( &service_thread, NULL, service_thread_entry, &io_service );
  wnd->show();
  qa.connect(&qa,SIGNAL(lastWindowClosed()),&qa,SLOT(quit()));
  qa.exec();
  io_service.stop();
  pthread_join(service_thread, NULL);

  return 0;

}
