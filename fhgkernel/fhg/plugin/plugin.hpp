#ifndef FHG_PLUGIN_HPP
#define FHG_PLUGIN_HPP 1

#include <errno.h>

#include <string>
#include <map>

#include <fhg/assert.hpp>
#include <fhg/plugin/descriptor.hpp>
#include <fhg/plugin/plugin_base.hpp>
#include <fhg/plugin/kernel.hpp>

typedef const fhg_plugin_descriptor_t* (*fhg_plugin_query)(void);
typedef fhg::plugin::Plugin*           (*fhg_plugin_create)(void);

typedef fhg::plugin::Kernel* FHG_KERNEL_PTR;
#define FHG_PLUGIN public fhg::plugin::Plugin

#define FHG_PLUGIN_START() int fhg_plugin_start ()
#define FHG_PLUGIN_STARTED() return 0
#define FHG_PLUGIN_FAILED(err) fhg_assert(err > 0); return -err

#define FHG_PLUGIN_STOP() int fhg_plugin_stop ()
#define FHG_PLUGIN_STOPPED() return 0
#define FHG_PLUGIN_BUSY() FHG_PLUGIN_FAILED(EBUSY)

#define FHG_ON_PLUGIN_LOADED(p) void fhg_on_plugin_loaded(std::string const &p)
#define FHG_ON_PLUGIN_UNLOAD(p) void fhg_on_plugin_unload(std::string const &p)
#define FHG_ON_PLUGIN_PREUNLOAD(p) void fhg_on_plugin_preunload(std::string const &p)

#ifdef FHG_STATIC_PLUGIN
#  define EXPORT_FHG_PLUGIN(name, cls, provides, desc, author, version, license, depends, key) \
  const fhg_plugin_descriptor_t *fhg_query_plugin_descriptor_##name()   \
  {                                                                     \
    static fhg_plugin_descriptor_t fhg_plugin_descriptor_##name =       \
      { #name,                                                          \
        desc,                                                           \
        author,                                                         \
        version,                                                        \
        __DATE__ " " __TIME__,                                          \
        license,                                                        \
        depends,                                                        \
        key,                                                            \
        provides                                                        \
      };                                                                \
    return &fhg_plugin_descriptor_##name;                               \
  }                                                                     \
  fhg::plugin::Plugin *fhg_get_plugin_instance_##name()                 \
  {                                                                     \
    return new cls();                                                   \
  }

#else

#  define EXPORT_FHG_PLUGIN(name, cls, provides, desc, author, version, license, depends, key) \
  extern "C"                                                            \
  {                                                                     \
    const fhg_plugin_descriptor_t *fhg_query_plugin_descriptor()        \
    {                                                                   \
      static fhg_plugin_descriptor_t fhg_plugin_descriptor_##name =     \
        { #name,                                                        \
          desc,                                                         \
          author,                                                       \
          version,                                                      \
          __DATE__ " " __TIME__,                                        \
          license,                                                      \
          depends,                                                      \
          key,                                                          \
          provides                                                      \
        };                                                              \
      return &fhg_plugin_descriptor_##name;                             \
    }                                                                   \
    fhg::plugin::Plugin *fhg_get_plugin_instance()                      \
    {                                                                   \
      return new cls();                                                 \
    }                                                                   \
  }

#endif

#endif
