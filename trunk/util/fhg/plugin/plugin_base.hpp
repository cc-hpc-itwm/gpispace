#ifndef FHG_PLUGIN_BASE_HPP
#define FHG_PLUGIN_BASE_HPP 1

namespace fhg
{
  namespace plugin
  {
    class Kernel;

    class Plugin
    {
    public:
      virtual ~Plugin(){}

      int fhg_plugin_start_entry (Kernel *k)
      {
        m_kernel = k;
        return fhg_plugin_start();
      }

      virtual int fhg_plugin_start () {return 0;}

      int fhg_plugin_stop_entry (Kernel *)
      {
        return fhg_plugin_stop();
      }
      virtual int fhg_plugin_stop  () {return 0;}

      virtual void fhg_on_plugin_loaded (std::string const & name) {}
      virtual void fhg_on_plugin_unload (std::string const & name) {}
      virtual void fhg_on_plugin_preunload (std::string const & name) {}
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
