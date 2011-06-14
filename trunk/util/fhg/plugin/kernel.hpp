#ifndef FHG_PLUGIN_KERNEL_HPP
#define FHG_PLUGIN_KERNEL_HPP 1

#include <vector>
#include <string>

#include <boost/signals2.hpp>

namespace fhg
{
  struct capability_t
  {
    std::string type;
    std::string version;
  };

  typedef std::vector<capability_t> capability_list_t;
  typedef boost::signals2::signal<void (capability_t const &)> capability_signal_t;

  class plugin;
  class kernel
  {
  public:
    virtual ~kernel() {}

    // signals
    capability_signal_t capability_gained;
    capability_signal_t capability_lost;

    virtual capability_list_t capabilities() const = 0;

    template <typename T>
    T* acquire_capability (std::string const & type)
    {
      return dynamic_cast<T*>(acquire(type));
    }
    template <typename T>
    T* acquire_capability (std::string const & type, std::string const &version)
    {
      return dynamic_cast<T*>(acquire(type, version));
    }

  protected:
    virtual void * acquire(std::string const & type) = 0;
    virtual void * acquire(std::string const & type, std::string const &version) = 0;
  };

  kernel & get_kernel ();
}

#endif
