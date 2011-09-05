#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <fhglog/remote/RemoteAppender.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>

#include <boost/archive/text_oarchive.hpp>

#include "observable.hpp"
#include "observer.hpp"
#include "task_event.hpp"

class GuiObserverPlugin : FHG_PLUGIN
                        , public observe::Observer
{
public:
  FHG_PLUGIN_START()
  {
    m_url = fhg_kernel()->get("url", "");
    if ("" == m_url)
    {
      MLOG(ERROR, "no GUI URL specified, please set plugin.gui.url");
      FHG_PLUGIN_FAILED(EINVAL);
    }

    try
    {
      m_destination = fhg::log::Appender::ptr_t
        (new fhg::log::remote::RemoteAppender ( "gui"
                                              , m_url
                                              )
        );
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not start appender to url: " << m_url << ": " << ex.what());
      FHG_PLUGIN_FAILED(EINVAL);
    }

    MLOG(INFO, "GUI sending events to " << m_url);

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    while (!m_observed.empty())
      stop_to_observe(*m_observed.begin());
    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    if (observe::Observable* o = fhg_kernel()->acquire<observe::Observable>(plugin))
    {
      MLOG(INFO, "GUI starts to observe: " << o << " (" << plugin << ")");
      m_observed.push_back(o);
      o->add_observer(this);
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

  void notify(boost::any const &evt)
  {
    try
    {
      task_event_t const & t (boost::any_cast<task_event_t>(evt));
      MLOG(TRACE, "*** TASK EVENT: " << t.id << " (" << t.name << ")" << " state = " << t.state);
      m_destination->append(FHGLOG_MKEVENT_HERE(INFO, encode(t)));
    }
    catch (boost::bad_any_cast const &ex)
    {
      MLOG(WARN, "got bad notification event: " << ex.what());
    }
    catch (std::exception const & ex)
    {
      MLOG(ERROR, "could not send event: " << ex.what());
    }
  }
private:
  void stop_to_observe(observe::Observable* o)
  {
    MLOG(INFO, "stopping to observe: " << o);
    o->del_observer(this);
    m_observed.remove(o);
  }

  static inline std::string encode (const task_event_t & e)
  {
    sdpa::daemon::NotificationEvent n_evt;
    n_evt.activity_id() = e.id;
    n_evt.activity_name() = e.name;
    n_evt.activity() = e.activity;
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

  std::list<observe::Observable*> m_observed;
  std::string m_url;
  fhg::log::Appender::ptr_t m_destination;
};

EXPORT_FHG_PLUGIN( gui
                 , GuiObserverPlugin
                 , "provides an gui-observer"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v.0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
