#ifndef FHG_PLUGIN_KERNEL_HPP
#define FHG_PLUGIN_KERNEL_HPP 1

#include <sstream>
#include <vector>
#include <string>

#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

namespace fhg
{
  namespace plugin
  {
    class Plugin;

    // this class only represents the interface available for a single plugin it
    // is implemented  by a mediator class  having access to the  plugin and the
    // real kernel,  therefore it's possible  to track dependencies by  calls to
    // acquire and release
    class Kernel
    {
    public:
      virtual ~Kernel() {}

      virtual std::string get(std::string const & key, std::string const &dflt) const = 0;

      template <typename T>
      T get(std::string const & key, std::string const &dflt) const
      {
        const std::string value (get(key, dflt));
        return boost::lexical_cast<T>(value);
      }

      template <typename T>
      T get(std::string const & key, T const & dflt) const
      {
        return get<T>(key, boost::lexical_cast<std::string>(dflt));
      }

      virtual int kill () = 0;
      virtual int shutdown () = 0;
      virtual int terminate () = 0;

      virtual std::string const & get_name () const = 0;
    };
  }
}

#endif
