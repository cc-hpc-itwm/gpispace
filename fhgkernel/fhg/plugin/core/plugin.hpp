#ifndef FHG_PLUGIN_CORE_PLUGIN_HPP
#define FHG_PLUGIN_CORE_PLUGIN_HPP

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <list>
#include <map>
#include <string>

namespace fhg
{
  namespace plugin
  {
    class Plugin;
  }

  namespace core
  {
    class kernel_t;
    class plugin_t : boost::noncopyable
    {
    public:
      plugin_t ( void * handle
               , kernel_t* kernel
               , std::list<boost::shared_ptr<plugin_t> > deps
               , std::map<std::string, std::string> config_variables
               );

    private:
      struct close_on_dtor_dlhandle
      {
        close_on_dtor_dlhandle (void*);
        ~close_on_dtor_dlhandle();
        void* _;

        template<typename T> T* sym (std::string);
      } m_handle;

      boost::scoped_ptr<fhg::plugin::Plugin> m_plugin;

      static std::list<plugin::Plugin*> to_raw (std::list<boost::shared_ptr<plugin_t> >);
    };
  }
}

#endif
