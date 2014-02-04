
#ifndef FHG_PLUGIN_HPP
#define FHG_PLUGIN_HPP 1

#include <errno.h>

#include <string>
#include <map>
#include <list>

#include <fhg/assert.hpp>
#include <fhg/plugin/descriptor.hpp>

#include <boost/utility.hpp>

#include <fhg/plugin/kernel.hpp>

#define FHG_PLUGIN public fhg::plugin::Plugin

#define FHG_PLUGIN_START() virtual void fhg_plugin_start ()

#define FHG_PLUGIN_STOP() virtual void fhg_plugin_stop ()

#define FHG_ON_PLUGIN_LOADED(p) virtual void fhg_on_plugin_loaded(Plugin* p)
#define FHG_ON_PLUGIN_PREUNLOAD(p) virtual void fhg_on_plugin_preunload(Plugin* p)

#define EXPORT_FHG_PLUGIN(name, cls, depends)                           \
  extern "C"                                                            \
  {                                                                     \
    const fhg_plugin_descriptor_t *fhg_query_plugin_descriptor()        \
    {                                                                   \
      static fhg_plugin_descriptor_t fhg_plugin_descriptor_##name =     \
        { #name,                                                        \
          depends,                                                      \
        };                                                              \
      return &fhg_plugin_descriptor_##name;                             \
    }                                                                   \
    fhg::plugin::Plugin *fhg_get_plugin_instance()                      \
    {                                                                   \
      return new cls();                                                 \
    }                                                                   \
  }

namespace fhg
{
  namespace plugin
  {
    class Kernel;

    class Plugin : boost::noncopyable
    {
    public:
      virtual ~Plugin(){}

      void fhg_plugin_start_entry (Kernel *k, std::list<Plugin*> deps)
      {
        m_kernel = k;
        m_dependencies = deps;
        fhg_plugin_start();
      }

      FHG_PLUGIN_START()
      {
      }
      FHG_PLUGIN_STOP()
      {
      }

#define EMPTY
      FHG_ON_PLUGIN_LOADED (EMPTY)
      {
      }
      FHG_ON_PLUGIN_PREUNLOAD (EMPTY)
      {
      }
#undef EMPTY

    protected:
      Plugin ()
        : m_kernel (0)
      {}

      Kernel *fhg_kernel() {return m_kernel;}
      std::list<Plugin*> m_dependencies;
    private:
      Kernel *m_kernel;
    };
  }
}

typedef const fhg_plugin_descriptor_t* (*fhg_plugin_query)(void);
typedef fhg::plugin::Plugin*           (*fhg_plugin_create)(void);

#endif
