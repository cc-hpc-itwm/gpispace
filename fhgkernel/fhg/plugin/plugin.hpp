#ifndef FHG_PLUGIN_HPP
#define FHG_PLUGIN_HPP

#include <string>
#include <map>
#include <list>

#include <fhg/assert.hpp>
#include <fhg/plugin/descriptor.hpp>
#include <fhg/plugin/kernel.hpp>

#include <boost/utility.hpp>

#define FHG_PLUGIN public fhg::plugin::Plugin

#define EXPORT_FHG_PLUGIN(name, cls, depends)                           \
  extern "C"                                                            \
  {                                                                     \
    const fhg_plugin_descriptor_t *fhg_query_plugin_descriptor()        \
    {                                                                   \
      static fhg_plugin_descriptor_t fhg_plugin_descriptor_##name =     \
        { #name,                                                        \
          depends,                                                      \
        };                                                              \
      return &fhg_plugin_descriptor_##name;                             \
    }                                                                   \
    fhg::plugin::Plugin *fhg_get_plugin_instance                        \
      (fhg::plugin::Kernel *k, std::list<fhg::plugin::Plugin*> deps)    \
    {                                                                   \
      return new cls (k, deps);                                         \
    }                                                                   \
  }

namespace fhg
{
  namespace plugin
  {
    class Plugin : boost::noncopyable
    {
    public:
      typedef fhg::plugin::Kernel Kernel;

      virtual ~Plugin(){}
    };
  }
}

#endif
