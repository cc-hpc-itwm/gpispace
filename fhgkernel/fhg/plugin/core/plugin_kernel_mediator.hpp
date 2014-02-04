#ifndef FHG_PLUGIN_PLUGIN_KERNEL_MEDIATOR_HPP
#define FHG_PLUGIN_PLUGIN_KERNEL_MEDIATOR_HPP 1

#include <set>

#include <fhg/plugin/kernel.hpp>
#include <fhg/plugin/core/plugin.hpp>

namespace fhg
{
  namespace core
  {
    class kernel_t; // the actual kernel

    class PluginKernelMediator : public fhg::plugin::Kernel
    {
    public:
      PluginKernelMediator (std::string name, kernel_t *kernel);

      std::string get(std::string const & key, std::string const &dflt) const;

      int shutdown ();

      std::string const & get_name () const;
    private:
      std::string _name;
      fhg::core::kernel_t *m_kernel;
    };
  }
}

#endif
