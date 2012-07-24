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

      class plugin_version_magic_mismatch : public plugin_exception
      {
      public:
        plugin_version_magic_mismatch ( std::string const & name
                                      , std::string const & got
                                      , std::string const & expected
                                      )
          : plugin_exception (name, "version mismatch for plugin: " + name + ": got := " + got + " expected := " + expected)
          , m_plugin_magic (got)
          , m_expected_magic (expected)
        {}

        ~plugin_version_magic_mismatch() throw () {}

        std::string const & plugin_magic () const { return m_plugin_magic; }
        std::string const & expected_magic () const { return m_expected_magic; }
      private:
        std::string m_plugin_magic;
        std::string m_expected_magic;
      };
    }
  }
}

#endif
