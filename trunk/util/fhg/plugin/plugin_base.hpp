#ifndef FHG_PLUGIN_BASE_HPP
#define FHG_PLUGIN_BASE_HPP 1

#include <fhg/plugin/config.hpp>

namespace fhg
{
  namespace plugin
  {
    class Plugin
    {
    public:
      virtual ~Plugin(){}

      virtual int fhg_plugin_start (config_t const &) { return 0; }
      virtual int fhg_plugin_stop  () { return 0; }
    };
  }
}

#endif
