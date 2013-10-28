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

typedef fhg::thread::channel<sdpa::daemon::NotificationEvent> event_channel_t;

class GuiObserverPlugin : FHG_PLUGIN
                        , public observe::Observer
{
public:
  FHG_PLUGIN_START()
  {
    m_thread = 0;

    const std::string url (fhg_kernel()->get ("url", ""));

    if (not url.empty ())
    {
      try
      {
        m_destination.reset (new fhg::log::ThreadedAppender
                            (fhg::log::Appender::ptr_t
                            (new fhg::log::remote::RemoteAppender("gui", url)
                            )));
        DMLOG (TRACE, "GUI sending events to " << url);
      }
      catch (std::exception const &ex)
      {
        MLOG(ERROR, "could not start appender to url: " << url << ": " << ex.what());
      }
    }

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
      m_events << boost::any_cast<sdpa::daemon::NotificationEvent>(evt);
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
      sdpa::daemon::NotificationEvent t = m_events.get ();

      DMLOG ( TRACE
            , "*** TASK EVENT:"
            << " id := " << t.activity_id()
            << " name := " << t.activity_name()
            << " state := " << t.activity_state()
            );

      try
      {
        m_destination->append (FHGLOG_MKEVENT_HERE (INFO, t.encoded()));
      }
      catch (std::exception const & ex)
      {
        MLOG(ERROR, "could not handle event: " << ex.what());
      }
    }
  }

  // void stop_to_observe(observe::Observable* o)
  // {
  //   MLOG(INFO, "stopping to observe: " << o);
  //   o->del_observer(this);
  //   m_observed.remove(o);
  // }

  fhg::log::Appender::ptr_t m_destination;

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
                 , ""
                 , ""
                 );
