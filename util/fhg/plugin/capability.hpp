#ifndef FHG_UTIL_PLUGIN_CAPABILITY_HPP
#define FHG_UTIL_PLUGIN_CAPABILITY_HPP 1

#include <string>

namespace fhg
{
  namespace plugin
  {
    class Capability
    {
    public:
      Capability (std::string const & name, std::string const & type)
        : m_name (name)
        , m_type (type)
      {}

      virtual ~Capability () {}

      virtual std::string const & capability_name () const
      {
        return m_name;
      };

      virtual std::string const & capability_type () const
      {
        return m_type;
      }
    private:
      std::string m_name;
      std::string m_type;
    };
  }
}

#endif
