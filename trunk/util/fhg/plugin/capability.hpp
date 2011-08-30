#ifndef FHG_UTIL_PLUGIN_CAPABILITY_HPP
#define FHG_UTIL_PLUGIN_CAPABILITY_HPP 1

namespace fhg
{
  namespace plugin
  {
    class Capability
    {
    public:
      virtual ~Capability () {}

      virtual const char * capability_name () const = 0;
      virtual const char * capability_type () const = 0;
    };
  }
}

#endif
