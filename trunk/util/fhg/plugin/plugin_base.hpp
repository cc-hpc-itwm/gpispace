#ifndef FHG_PLUGIN_BASE_HPP
#define FHG_PLUGIN_BASE_HPP 1

namespace fhg
{
  namespace plugin
  {
    class Kernel;

    class Plugin
    {
    public:
      virtual ~Plugin(){}

      virtual int fhg_plugin_start (Kernel*) { return 0; }
      virtual int fhg_plugin_stop  (Kernel*) { return 0; }
    };
  }
}

#endif
