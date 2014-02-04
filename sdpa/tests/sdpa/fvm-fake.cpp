#include <fhglog/LogMacros.hpp>
#include <fhg/plugin/plugin.hpp>
#include <boost/lexical_cast.hpp>

#include <fvm-pc/pc.hpp>

using boost::lexical_cast;

class FvmFakePluginImpl : FHG_PLUGIN
{
public:
  FvmFakePluginImpl (Kernel *k, std::list<Plugin*> deps)
    : Plugin (k, deps)
  {
    size_t fvm_size (100*1024*1024);
    size_t shm_size (50*1024*1024);

    fvm_size = lexical_cast<size_t>( fhg_kernel()->get( "fvm_size"
                                                      , lexical_cast<std::string>(fvm_size)
                                                      )
                                   );

    shm_size = lexical_cast<size_t>( fhg_kernel()->get( "shm_size"
                                                      , lexical_cast<std::string>(shm_size)
                                                      )
                                   );

    int ec = fvmConnect(fvm_pc_config_t("/dummy", "/dummy", shm_size, fvm_size));

    if (0 != ec)
    {
      throw std::runtime_error
        ("fvmConnect failed: " + std::string (strerror (-ec)));
    }
  }

  FHG_PLUGIN_STOP()
  {
    fvmLeave();
  }
};

EXPORT_FHG_PLUGIN (fvm_fake, FvmFakePluginImpl, "");
