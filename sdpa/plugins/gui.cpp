#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <sdpa/daemon/NotificationEvent.hpp>
#include <sdpa/daemon/NotificationService.hpp>

#include <boost/thread.hpp>

#include "observable.hpp"
#include "observer.hpp"

class GuiObserverPlugin : FHG_PLUGIN
                        , public observe::Observer
{
public:
  FHG_PLUGIN_START()
  {
    _notification_service = boost::none;

    const std::string url (fhg_kernel()->get ("url", ""));
    if (not url.empty ())
    {
      _notification_service = sdpa::daemon::NotificationService (url);
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    observe::Observer::stop_to_observe();

    _notification_service = boost::none;

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
      if (_notification_service)
      {
        _notification_service->notify
          (boost::any_cast<sdpa::daemon::NotificationEvent>(evt));
      }
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
  boost::optional<sdpa::daemon::NotificationService> _notification_service;
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
