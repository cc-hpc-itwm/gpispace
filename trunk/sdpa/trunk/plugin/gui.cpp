#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include "observable.hpp"
#include "observer.hpp"

class GuiObserverPlugin : FHG_PLUGIN
                        , public observe::Observer
{
public:
  FHG_PLUGIN_START()
  {
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
    MLOG(INFO, "got notification event: " << boost::any_cast<std::string>(evt));
  }
private:
  void stop_to_observe(observe::Observable* o)
  {
    MLOG(INFO, "stopping to observe: " << o);
    o->del_observer(this);
    m_observed.remove(o);
  }

  std::list<observe::Observable*> m_observed;
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
