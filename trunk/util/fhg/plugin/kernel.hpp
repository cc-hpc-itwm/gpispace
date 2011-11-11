#ifndef FHG_PLUGIN_KERNEL_HPP
#define FHG_PLUGIN_KERNEL_HPP 1

#include <sstream>
#include <vector>
#include <string>

#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

#include <fhg/plugin/plugin_base.hpp>
#include <fhg/plugin/storage.hpp>

namespace fhg
{
  namespace plugin
  {
    typedef boost::function<void (void)> task_t;

    namespace exception
    {
      struct config_error : public std::runtime_error
      {
        config_error (std::string const &k, std::string const &v, std::string const &m)
          : std::runtime_error ("config_error: key := " + k + " value := `" + v + "' - " + m)
          , m_key (k)
          , m_val (v)
        {}

        ~config_error () throw () {}

        std::string const & key () const { return m_key; }
        std::string const & value () const { return m_val; }
      private:
        std::string m_key;
        std::string m_val;
      };
    }

    // this class only represents the interface available for a single plugin it
    // is implemented  by a mediator class  having access to the  plugin and the
    // real kernel,  therefore it's possible  to track dependencies by  calls to
    // acquire and release
    class Kernel
    {
    public:
      virtual ~Kernel() {}

      template <typename Iface>
      Iface* acquire (std::string const & name)
      {
        Iface* iface = dynamic_cast<Iface*>(this->acquire(name));
        if (0 == iface)
        {
          this->release(name);
        }
        return iface;
      }
      virtual Plugin * acquire(std::string const & name) = 0;
      virtual void     release(std::string const & name) = 0;

      virtual Storage *storage() = 0;

      virtual void schedule(std::string const & name, task_t) = 0;
      virtual void schedule(std::string const & name, task_t, size_t ticks) = 0;

      virtual std::string get(std::string const & key, std::string const &dflt) const = 0;

      template <typename T>
      T get(std::string const & key, std::string const &dflt) const
      {
        const std::string value (get(key, dflt));
        try
        {
          return boost::lexical_cast<T>(value);
        }
        catch (std::exception const & ex)
        {
          throw exception::config_error (key, value, ex.what());
        }
      }

      virtual void start_completed(int) = 0;

      virtual int load_plugin (std::string const &path) = 0;
      virtual int unload_plugin (std::string const &name) = 0;

      virtual int terminate () = 0;
    };
  }
}

#endif
