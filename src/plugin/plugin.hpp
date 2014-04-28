#ifndef FHG_PLUGIN_HPP
#define FHG_PLUGIN_HPP

#include <string>
#include <map>
#include <list>

#include <fhg/assert.hpp>

#include <boost/utility.hpp>

#include <functional>

#define FHG_PLUGIN public fhg::plugin::Plugin

struct fhg_plugin_descriptor_t
{
  const char *name;
  const char *depends;
};

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
    fhg::plugin::Plugin *fhg_get_plugin_instance                        \
      (std::function<void()> request_stop, std::list<fhg::plugin::Plugin*> deps, std::map<std::string, std::string> config_variables) \
    {                                                                   \
      return new cls (request_stop, deps, config_variables);            \
    }                                                                   \
  }

namespace fhg
{
  namespace plugin
  {
    class Plugin : boost::noncopyable
    {
    public:
      virtual ~Plugin() = default;
    };
  }
}

#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

//! \note Temporary, while config_variables are passed in as map<>.
template<typename T> boost::optional<T> get
  (std::string key, std::map<std::string, std::string> const& vals)
{
  const std::map<std::string, std::string>::const_iterator it (vals.find (key));
  if (it != vals.end())
  {
    return boost::lexical_cast<T> (it->second);
  }
  return boost::none;
}

#endif
