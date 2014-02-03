#ifndef FHG_PLUGIN_CORE_EXCEPTION_HPP
#define FHG_PLUGIN_CORE_EXCEPTION_HPP 1

#include <string>
#include <stdexcept>

namespace fhg
{
  namespace core
  {
    namespace exception
    {
      class plugin_exception : public std::runtime_error
      {
      public:
        plugin_exception (std::string const &name, std::string const &msg)
          : std::runtime_error (msg)
          , m_plugin_name (name)
        {}

        virtual ~plugin_exception () throw () {}

        std::string const & plugin_name () const { return m_plugin_name; }
      private:
        std::string m_plugin_name;
      };
    }
  }
}

#endif
