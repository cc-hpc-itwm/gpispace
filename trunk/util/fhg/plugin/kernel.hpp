#ifndef FHG_PLUGIN_KERNEL_HPP
#define FHG_PLUGIN_KERNEL_HPP 1

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

      template <typename T>
      T* acquire_plugin (std::string const & name)
      {
        return dynamic_cast<T*>(this->acquire(name));
      }
      virtual Plugin * acquire(std::string const & name) = 0;
      virtual void     release(std::string const & name) = 0;

      virtual void schedule_immediate(task_t) = 0;
      virtual void schedule_later(task_t, size_t ticks) = 0;
    };
  }
}

#endif
