#ifndef FHG_PLUGIN_CORE_PLUGIN_HPP
#define FHG_PLUGIN_CORE_PLUGIN_HPP 1

#include <list>
#include <string>

#include <boost/shared_ptr.hpp>
#include <fhg/plugin/build.hpp>
#include <fhg/plugin/plugin_base.hpp>

namespace fhg
{
  namespace core
  {
    class plugin_t
    {
    public:
      typedef boost::shared_ptr<plugin_t> ptr_t;

      ~plugin_t ();

      static ptr_t create (std::string const & filename, bool force);

      std::string const & name () const;

      const fhg_plugin_descriptor_t * descriptor() const;
      fhg::plugin::Plugin * get_plugin();
      const fhg::plugin::Plugin * get_plugin() const;

      size_t use_count() const;
      void used_by   (const plugin_t *p);
      bool is_used_by (const plugin_t *p) const;
      void unused_by (const plugin_t *p);

      int start (fhg::plugin::config_t const &);
      int stop ();

      template <typename T>
      T* as ()
      {
        return dynamic_cast<T*>(get_plugin());
      }

      template <typename T>
      const T* as () const
      {
        return dynamic_cast<const T*>(get_plugin());
      }
    private:
      typedef std::list<const plugin_t*> used_by_list_t;

      void check_dependencies();

      plugin_t ( std::string const & name
               , std::string const & filename
               , fhg::plugin::Plugin *
               , const fhg_plugin_descriptor_t *
               , int flags
               , void * handle
               );

      std::string m_name;
      std::string m_file_name;
      fhg::plugin::Plugin *m_plugin;
      const fhg_plugin_descriptor_t *m_descriptor;
      int m_flags;
      void *m_handle;

      used_by_list_t m_used_by;
    };
  }
}

//extern void registerStaticPluginCreator(const char *, void* (*creator)());

#endif
