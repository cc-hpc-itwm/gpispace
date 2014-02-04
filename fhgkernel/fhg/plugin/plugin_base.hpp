#ifndef FHG_PLUGIN_BASE_HPP
#define FHG_PLUGIN_BASE_HPP 1

#include <boost/utility.hpp>

namespace fhg
{
  namespace plugin
  {
    class Kernel;

    class Plugin : boost::noncopyable
    {
    public:
      virtual ~Plugin(){}

      int fhg_plugin_start_entry (Kernel *k)
      {
        m_kernel = k;
        return fhg_plugin_start();
      }

      virtual int fhg_plugin_start () {return 0;}

      virtual int fhg_plugin_stop  () {return 0;}

      virtual void fhg_on_plugin_loaded (Plugin*) {}
      virtual void fhg_on_plugin_preunload (Plugin*) {}
    protected:
      Plugin ()
        : m_kernel (0)
      {}

      Kernel *fhg_kernel() {return m_kernel;}
    private:
      Kernel *m_kernel;
    };
  }
}

#endif
