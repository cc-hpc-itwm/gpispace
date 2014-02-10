#include <fhg/plugin/plugin.hpp>

#include <fvm-pc/pc.hpp>

class FvmFakePluginImpl : FHG_PLUGIN
{
public:
  FvmFakePluginImpl (boost::function<void()>, std::list<Plugin*>, std::map<std::string, std::string> config_variables)
  {
    const size_t fvm_size (get<size_t> ("plugin.fvm_fake.fvm_size", config_variables).get_value_or (100 << 20));
    const size_t shm_size (get<size_t> ("plugin.fvm_fake.shm_size", config_variables).get_value_or (50 << 20));

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
