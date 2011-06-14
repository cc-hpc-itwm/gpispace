#ifndef FHG_PLUGIN_KERNEL_HPP
#define FHG_PLUGIN_KERNEL_HPP 1

#include <vector>

namespace fhg
{
  typedef std::vector<std::string> name_list_t;

  class plugin;
  class kernel
  {
  public:
    virtual ~kernel() {}

    virtual name_list_t capabilities() const = 0;

    virtual plugin * acquire_capability (std::string const & type) = 0;
    virtual plugin * acquire_capability (std::string const & type, std::string const &impl) = 0;
  };
}

#endif
