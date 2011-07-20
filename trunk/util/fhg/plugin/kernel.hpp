#ifndef FHG_PLUGIN_KERNEL_HPP
#define FHG_PLUGIN_KERNEL_HPP 1

#include <vector>
#include <string>

#include <boost/signals2.hpp>
#include <boost/function.hpp>

namespace fhg
{
  namespace plugin
  {
    struct plugin_info_t
    {
      std::string name;
      std::string type;
      std::string version;
    };

    typedef boost::function<void (void)> task_t;
    typedef std::vector<plugin_info_t> plugin_info_list_t;
    typedef boost::signals2::signal<void (plugin_info_t const &)> plugin_signal_t;

    class kernel_t
    {
    public:
      virtual ~kernel_t() {}

      // signals
      plugin_signal_t plugin_loaded;
      plugin_signal_t plugin_unloaded;

      virtual plugin_info_list_t list_plugins() const = 0;

      template <typename T>
      T* acquire_plugin (std::string const & name)
      {
        return dynamic_cast<T*>(acquire(name));
      }
      template <typename T>
      T* acquire_plugin (std::string const & name, std::string const &version)
      {
        return dynamic_cast<T*>(acquire(name, version));
      }

      void release_plugin (void *c)
      {
        release (c);
      }

      virtual int load_plugin (std::string const & file) = 0;
      virtual int unload_plugin (std::string const &name) = 0;

      virtual void schedule(task_t) = 0;
    protected:
      virtual void * acquire(std::string const & name) = 0;
      virtual void * acquire(std::string const & name, std::string const &version) = 0;
      virtual void   release(void *) = 0;
    };

    extern kernel_t * get_kernel ();
  }
}

#endif
