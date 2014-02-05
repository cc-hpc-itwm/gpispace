#include <fhg/plugin/plugin.hpp>

#include <fvm-pc/pc.hpp>

class FvmFakePluginImpl : FHG_PLUGIN
{
public:
  FvmFakePluginImpl (Kernel*, std::list<Plugin*>, std::map<std::string, std::string> config_variables)
  {
    size_t fvm_size (100*1024*1024);
    size_t shm_size (50*1024*1024);

    fvm_size = get<size_t> ("plugin.fvm_fake.fvm_size", config_variables).get_value_or (fvm_size);
    shm_size = get<size_t> ("plugin.fvm_fake.shm_size", config_variables).get_value_or (shm_size);

    int ec = fvmConnect(fvm_pc_config_t("/dummy", "/dummy", shm_size, fvm_size));

    if (0 != ec)
    {
      throw std::runtime_error
        ("fvmConnect failed: " + std::string (strerror (-ec)));
    }
  }

  ~FvmFakePluginImpl()
  {
    fvmLeave();
  }
};

EXPORT_FHG_PLUGIN (fvm_fake, FvmFakePluginImpl, "");
