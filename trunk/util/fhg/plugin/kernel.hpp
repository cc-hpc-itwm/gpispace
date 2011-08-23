#ifndef FHG_PLUGIN_KERNEL_HPP
#define FHG_PLUGIN_KERNEL_HPP 1

#include <sstream>
#include <vector>
#include <string>

#include <boost/function.hpp>

#include <fhg/plugin/plugin_base.hpp>

namespace fhg
{
  namespace plugin
  {
    typedef boost::function<void (void)> task_t;

    // this class only represents the interface available for a single plugin it
    // is implemented  by a mediator class  having access to the  plugin and the
    // real kernel,  therefore it's possible  to track dependencies by  calls to
    // acquire and release
    class Kernel
    {
    public:
      virtual ~Kernel() {}

      template <typename Iface>
      Iface* acquire (std::string const & name)
      {
        Iface* iface = dynamic_cast<Iface*>(this->acquire(name));
        if (0 == iface)
        {
          this->release(name);
        }
        return iface;
      }
      virtual Plugin * acquire(std::string const & name) = 0;
      virtual void     release(std::string const & name) = 0;

      virtual void schedule(task_t) = 0;
      virtual void schedule(task_t, size_t ticks) = 0;

      virtual std::string get(std::string const & key, std::string const &dflt) const = 0;
      template <typename T>
      T get(std::string const & key, std::string const &dflt) const
      {
        std::stringstream s (get(key, dflt));
        T v;
        s >> v;
        return v;
      }
    };
  }
}

#endif
