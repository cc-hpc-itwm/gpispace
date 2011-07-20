#ifndef FHG_PLUGIN_KERNEL_HPP
#define FHG_PLUGIN_KERNEL_HPP 1

#include <vector>
#include <string>

#include <boost/signals2.hpp>
#include <boost/function.hpp>

#include <fhg/plugin/plugin_base.hpp>

namespace fhg
{
  namespace plugin
  {
    typedef boost::signals2::signal<void (std::string const &)> plugin_signal_t;
    typedef boost::function<void (void)> task_t;

    // this class only represents the interface available for a single plugin it
    // is implemented  by a mediator class  having access to the  plugin and the
    // real kernel,  therefore it's possible  to track dependencies by  calls to
    // acquire and release
    class Kernel
    {
    public:
      virtual ~Kernel() {}

      // signals
      plugin_signal_t plugin_loaded;
      plugin_signal_t plugin_unloaded;

      template <typename T>
      T* acquire_plugin (std::string const & name)
      {
        return dynamic_cast<T*>(this->acquire(name));
      }
      virtual Plugin * acquire(std::string const & name) = 0;
      virtual void   release(Plugin *) = 0;

      virtual void schedule(task_t, unsigned long millis_from_now = 0) = 0;
    };
  }
}

#endif
