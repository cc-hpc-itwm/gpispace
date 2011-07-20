#ifndef FHG_PLUGIN_HPP
#define FHG_PLUGIN_HPP 1

#include <string>
#include <map>

#include <fhg/plugin/build.hpp>
#include <fhg/plugin/config.hpp>

namespace fhg
{
  namespace plugin
  {
    class Plugin
    {
    public:
      virtual ~Plugin(){}

      virtual int fhg_plugin_start (config_t const &) { return 0; }
      virtual int fhg_plugin_stop  () { return 0; }
    };
  }
}

typedef const fhg_plugin_descriptor_t* (*fhg_plugin_query)(void);
typedef fhg::plugin::Plugin*           (*fhg_plugin_create)(void);

#define IS_A_FHG_PLUGIN public fhg::plugin::Plugin

#ifdef FHG_STATIC_PLUGIN

#  define FHG_PLUGIN(name, cls, desc, author, version, license, depends, key) \
  extern "C"                                                            \
  {                                                                     \
    const fhg_plugin_descriptor_t *fhg_query_plugin_descriptor_##name() \
    {                                                                   \
      static fhg_plugin_descriptor_t fhg_plugin_descriptor_##name =   \
        {#name,desc,author,version,license,depends,key,FHG_PLUGIN_VERSION_MAGIC}; \
      return &fhg_plugin_descriptor_##name;                             \
    }                                                                   \
    fhg::plugin::Plugin *fhg_get_plugin_instance_##name()               \
    {                                                                   \
      static cls* fhg_plugin_instance_##name = 0;                       \
      if (0 == fhg_plugin_instance_##name)                              \
      {                                                                 \
        fhg_plugin_instance_##name = new cls();                         \
      }                                                                 \
      return fhg_plugin_instance_##name;                                \
    }                                                                   \
  }

#else

#  define FHG_PLUGIN(name, cls, desc, author, version, license, depends, key) \
  extern "C"                                                            \
  {                                                                     \
    const fhg_plugin_descriptor_t *fhg_query_plugin_descriptor()        \
    {                                                                   \
      static fhg_plugin_descriptor_t fhg_plugin_descriptor =            \
        {#name,desc,author,version,license,depends,key,FHG_PLUGIN_VERSION_MAGIC}; \
      return &fhg_plugin_descriptor;                                    \
    }                                                                   \
    fhg::plugin::Plugin *fhg_get_plugin_instance()                      \
    {                                                                   \
      static cls* instance = 0;                                         \
      if (0 == instance)                                                \
      {                                                                 \
        instance = new cls();                                           \
      }                                                                 \
      return instance;                                                  \
    }                                                                   \
  }

#endif

#define FHG_IMPORT_PLUGIN(name)                                         \
  extern void fhgRegisterStaticPlugin(fhg_plugin_query, fhg_plugin_create); \
  extern const fhg_plugin_descriptor_t *fhg_query_plugin_descriptor_##name(); \
  extern fhg::plugin::Plugin *fhg_get_plugin_instance_##name();         \
  struct fhgStatic_##name##_initializer_t                               \
  {                                                                     \
    fhgStatic_##name##_initializer_t ()                                 \
    {                                                                   \
      fhgRegisterStaticPlugin(fhg_query_plugin_descriptor_##name, fhg_get_plugin_instance_##name); \
    }                                                                   \
    ~fhgStatic_##name##_initializer_t () {}                             \
  };                                                                    \
  static fhgStatic_##name##_initializer_t __fhgStatic_initializer_##name

#endif
