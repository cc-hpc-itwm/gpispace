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
        return fhg_plugin_start(k);
      }

      virtual int fhg_plugin_start (Kernel*) {return 0;}

      int fhg_plugin_stop_entry (Kernel *k)
      {
        return fhg_plugin_stop(k);
      }
      virtual int fhg_plugin_stop  (Kernel*) {return 0;}

      virtual void fhg_on_plugin_loaded (std::string const & name) {}
      virtual void fhg_on_plugin_unload (std::string const & name) {}
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
