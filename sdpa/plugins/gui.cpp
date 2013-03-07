#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <fhg/util/thread/channel.hpp>
#include <fhglog/ThreadedAppender.hpp>
#include <fhglog/remote/RemoteAppender.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <boost/thread.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "observable.hpp"
#include "observer.hpp"
#include "task_event.hpp"

#include "kvs.hpp"

typedef fhg::thread::channel<task_event_t> event_channel_t;

class GuiObserverPlugin : FHG_PLUGIN
                        , public observe::Observer
{
public:
  FHG_PLUGIN_START()
  {
    m_thread = 0;

    m_kvs_prefix =
      "job." + fhg_kernel ()->get_name () + ".current";

    m_url = fhg_kernel()->get("url", "");
    if ("" == m_url)
    {
      MLOG(ERROR, "no GUI URL specified, please set plugin.gui.url");
      FHG_PLUGIN_FAILED(EINVAL);
    }

    try
    {
      m_destination.reset (new fhg::log::ThreadedAppender
                          (fhg::log::Appender::ptr_t
                          (new fhg::log::remote::RemoteAppender( "gui"
                                                               , m_url
                                                               )
                          )));
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not start appender to url: " << m_url << ": " << ex.what());
      FHG_PLUGIN_FAILED(EINVAL);
    }

    MLOG(INFO, "GUI sending events to " << m_url);

    m_kvs = fhg_kernel ()->acquire<kvs::KeyValueStore>("kvs");

    m_thread =
      new boost::thread
      (boost::bind( &GuiObserverPlugin::event_handler
                  , this
                  )
      );

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    observe::Observer::stop_to_observe();

    if (m_thread)
    {
      m_thread->interrupt ();
      m_thread->join ();
      delete m_thread;
    }

    m_destination.reset();
    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    if (observe::Observable* o = fhg_kernel()->acquire<observe::Observable>(plugin))
    {
      MLOG(INFO, "GUI starts to observe: " << o << " (" << plugin << ")");
      start_to_observe(o);
    }
  }

  FHG_ON_PLUGIN_UNLOAD(plugin)
  {
  }

  FHG_ON_PLUGIN_PREUNLOAD(plugin)
  {
    if (observe::Observable* o = fhg_kernel()->acquire<observe::Observable>(plugin))
    {
      stop_to_observe(o);
      fhg_kernel()->release(plugin);
    }
  }

  void notify(const observe::Observable *, boost::any const &evt)
  {
    try
    {
      m_events << boost::any_cast<task_event_t>(evt);
    }
    catch (boost::bad_any_cast const &ex)
    {
    }
    catch (std::exception const & ex)
    {
      MLOG(ERROR, "could not notify event: " << ex.what());
    }
  }
private:
  void event_handler ()
  {
    for (;;)
    {
      task_event_t t = m_events.get ();

      MLOG ( TRACE
           , "*** TASK EVENT:"
           << " id := " << t.id
           << " name := " << t.name
           << " state := " << t.state
           << " time := " << t.tstamp
           );

      try
      {
        m_destination->append(FHGLOG_MKEVENT_HERE(INFO, encode(t)));
        // store task in kvs
        store_task_info_to_kvs (t);
      }
      catch (std::exception const & ex)
      {
        MLOG(ERROR, "could not handle event: " << ex.what());
      }
    }
  }

  void store_task_info_to_kvs (task_event_t const & t)
  {
    m_kvs->put (m_kvs_prefix + "." + "id",     t.id);
    m_kvs->put (m_kvs_prefix + "." + "name",   t.name);
    m_kvs->put (m_kvs_prefix + "." + "state",  t.state);
    m_kvs->put (m_kvs_prefix + "." + "tstamp", t.tstamp);
  }

  // void stop_to_observe(observe::Observable* o)
  // {
  //   MLOG(INFO, "stopping to observe: " << o);
  //   o->del_observer(this);
  //   m_observed.remove(o);
  // }

  static inline std::string encode (const task_event_t & e)
  {
    sdpa::daemon::NotificationEvent n_evt;
    n_evt.activity_id() = e.id;
    n_evt.activity_name() = e.name;
    n_evt.activity() = e.activity;
    if (e.meta.find("agent.name") != e.meta.end())
    {
      n_evt.component() = e.meta.find("agent.name")->second;
    }
    else
    {
      n_evt.component() = "unknown";
    }
    switch (e.state)
    {
    case task_event_t::ENQUEUED:
      n_evt.activity_state() = sdpa::daemon::NotificationEvent::STATE_CREATED;
      break;
    case task_event_t::DEQUEUED:
      n_evt.activity_state() = sdpa::daemon::NotificationEvent::STATE_STARTED;
      break;
    case task_event_t::FINISHED:
      n_evt.activity_state() = sdpa::daemon::NotificationEvent::STATE_FINISHED;
      break;
    case task_event_t::FAILED:
      n_evt.activity_state() = sdpa::daemon::NotificationEvent::STATE_FAILED;
      break;
    case task_event_t::CANCELED:
      n_evt.activity_state() = sdpa::daemon::NotificationEvent::STATE_CANCELLED;
      break;
    default:
      n_evt.activity_state() = sdpa::daemon::NotificationEvent::STATE_IGNORE;
      break;
    }

    std::ostringstream sstr;
    boost::archive::text_oarchive ar(sstr);
    ar << n_evt;
    return sstr.str();
  }

  std::string m_url;
  fhg::log::Appender::ptr_t m_destination;
  std::string m_kvs_prefix;
  kvs::KeyValueStore *m_kvs;
  event_channel_t m_events;
  boost::thread *m_thread;
};

EXPORT_FHG_PLUGIN( gui
                 , GuiObserverPlugin
                 , "gui"
                 , "provides an gui-observer"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "kvs"
                 , ""
                 );
