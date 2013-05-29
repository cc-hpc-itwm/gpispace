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

    if (not m_url.empty ())
    {
      try
      {
        m_destination.reset (new fhg::log::ThreadedAppender
                            (fhg::log::Appender::ptr_t
                            (new fhg::log::remote::RemoteAppender( "gui"
                                                                 , m_url
                                                                 )
                            )));
        DMLOG (TRACE, "GUI sending events to " << m_url);
      }
      catch (std::exception const &ex)
      {
        MLOG(ERROR, "could not start appender to url: " << m_url << ": " << ex.what());
      }
    }

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
      DMLOG(TRACE, "GUI starts to observe: " << o << " (" << plugin << ")");
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
    std::ostringstream sstr;
    boost::archive::text_oarchive ar (sstr);

    const sdpa::daemon::NotificationEvent evt
      ( e.meta.find("agent.name") != e.meta.end()
      ? e.meta.find("agent.name")->second
      : "unknown"
      , e.id
      , e.name
      , e.state == task_event_t::ENQUEUED ? sdpa::daemon::NotificationEvent::STATE_CREATED
      : e.state == task_event_t::DEQUEUED ? sdpa::daemon::NotificationEvent::STATE_STARTED
      : e.state == task_event_t::FINISHED ? sdpa::daemon::NotificationEvent::STATE_FINISHED
      : e.state == task_event_t::FAILED ? sdpa::daemon::NotificationEvent::STATE_FAILED
      : e.state == task_event_t::CANCELED ? sdpa::daemon::NotificationEvent::STATE_CANCELLED
      : sdpa::daemon::NotificationEvent::STATE_IGNORE
      , e.activity
      );

    ar << evt;

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
