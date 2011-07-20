#ifndef FHG_PLUGIN_CONFIGURABLE
#define FHG_PLUGIN_CONFIGURABLE 1

#include <map>
#include <string>

namespace fhg
{
  namespace plugin
  {
    typedef std::map<std::string, std::string> config_t;

    class Configurable
    {
    public:
      virtual ~Configurable() {}
      virtual void configure(config_t const &) = 0;
    };
  }
}

#endif
