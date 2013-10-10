#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include "wfe.hpp"

class WfeEvalPluginImpl : FHG_PLUGIN
{
public:
  virtual ~WfeEvalPluginImpl() {}

  FHG_PLUGIN_START()
  {
    wfe::WFE * wfe = fhg_kernel()->acquire<wfe::WFE>("wfe");

    const std::string input(fhg_kernel()->get ("input", "-"));
    const std::string output(fhg_kernel()->get ("output", "-"));

    std::stringstream sstr;
    if (input == "-")
    {
      std::cin >> std::noskipws >> sstr.rdbuf();
    }
    else
    {
      std::ifstream ifs(input.c_str());
      ifs >> std::noskipws >> sstr.rdbuf();
    }

    const std::string workflow_description (sstr.str());
    std::string       workflow_result;
    std::string       error_message;

    std::list<std::string> worker_list;
    worker_list.push_back ("worker-localhost-1");

    int ec = wfe->execute ( "job-1"
                          , workflow_description
                          , wfe::capabilities_t ()
                          , workflow_result
                          , error_message
                          , worker_list
                          );

    if (output == "-")
    {
      std::cout << workflow_result << std::endl;
    }
    else
    {
      std::ofstream ofs (output.c_str());
      ofs << workflow_result << std::endl;
    }

    exit (ec);

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }
};

EXPORT_FHG_PLUGIN( wfe_eval
                 , WfeEvalPluginImpl
                 , ""
                 , "provides an easy way to evaluate a workflow in a fhgkernel context"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "wfe"
                 , ""
                 );
