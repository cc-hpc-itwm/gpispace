#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <boost/lexical_cast.hpp>

#include <fvm-pc/pc.hpp>

using boost::lexical_cast;

class FvmFakePluginImpl : FHG_PLUGIN
{
public:
  FHG_PLUGIN_START()
  {
    size_t fvm_size (100*1024*1024);
    size_t shm_size (50*1024*1024);

    try
    {
      fvm_size = lexical_cast<size_t>( fhg_kernel()->get( "fvm_size"
                                                        , lexical_cast<std::string>(fvm_size)
                                                        )
                                     );
    }
    catch (std::exception const &ex)
    {
      LOG(ERROR, "could not parse plugin.fvm-fake.fvm_size: " << ex.what());
      FHG_PLUGIN_FAILED(EINVAL);
    }

    try
    {
      shm_size = lexical_cast<size_t>( fhg_kernel()->get( "shm_size"
                                                        , lexical_cast<std::string>(shm_size)
                                                        )
                                     );
    }
    catch (std::exception const &ex)
    {
      LOG(ERROR, "could not parse plugin.fvm-fake.shm_size: " << ex.what());
      FHG_PLUGIN_FAILED(EINVAL);
    }

    int ec = fvmConnect(fvm_pc_config_t("/dummy", "/dummy", shm_size, fvm_size));

    if (0 != ec)
    {
      FHG_PLUGIN_FAILED(-ec);
    }

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    fvmLeave();
    FHG_PLUGIN_STOPPED();
  }
};

EXPORT_FHG_PLUGIN( fvm_fake
                 , FvmFakePluginImpl
                 , ""
                 , "provides a fake implementation for the deprecated fvm-pc library"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
