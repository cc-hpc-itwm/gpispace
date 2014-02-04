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

      virtual void stop() = 0;

      virtual std::string const & get_name () const = 0;
    };
  }
}

#endif
