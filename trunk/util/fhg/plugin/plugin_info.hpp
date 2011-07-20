#ifndef FHG_PLUGIN_INFO
#define FHG_PLUGIN_INFO 1

#include <vector>
#include <string>

namespace fhg
{
  namespace plugin
  {
    class PluginInfo
    {
    public:
      virtual ~PluginInfo() {}

      virtual std::string const & type () const = 0;
      virtual std::string const & info () const = 0;
      //    virtual std::vector<std::string> const & dependencies() const = 0;
    };
  }
}

#endif
