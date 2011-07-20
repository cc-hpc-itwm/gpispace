#ifndef FHG_PLUGIN_CORE_PLUGIN_HPP
#define FHG_PLUGIN_CORE_PLUGIN_HPP 1

#include <boost/shared_ptr.hpp>

#include <fhg/plugin/plugin_info.hpp>
#include <fhg/plugin/configurable.hpp>
#include <fhg/plugin/startable.hpp>
#include <fhg/plugin/stoppable.hpp>

namespace fhg
{
  namespace core
  {
    class plugin_t : public fhg::plugin::Configurable
                   , public fhg::plugin::Startable
                   , public fhg::plugin::Stoppable
                   , public fhg::plugin::PluginInfo
    {
    public:
      ~plugin_t ();

      static plugin_t * create (std::string const & filename);

      std::string const & name () const;

      size_t use_count() const;
      void used_by   (const plugin_t *p);
      void unused_by (const plugin_t *p);

      void check_dependencies();

      // plugin info
      std::string const & type () const;
      std::string const & info () const;

      // configurable
      void configure (fhg::plugin::config_t const &);

      // startable
      void start ();

      // stoppable
      void stop  ();

    private:
      typedef std::list<plugin_t*> used_by_list_t;

      plugin_t ( std::string const & name
               , std::string const & filename
               , void *handle
               , int flags
               );

      std::string m_file_name;
      void *m_handle;

      used_by_list_t m_used_by;
    };
  }
}

//extern void registerStaticPluginCreator(const char *, void* (*creator)());

#endif
