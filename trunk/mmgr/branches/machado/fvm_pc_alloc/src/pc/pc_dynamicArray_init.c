#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>


#include <modules.hpp>
#include <iostream>
#include <Module.hpp>
#include <ModuleLoader.hpp>

#include <pc.h>

using namespace std;
using namespace sdpa;


//test that init the dynamic array
//prints out the handle to be used by the Usage test
// the handle returned can be used by the Usage test on different nodes
static void usage(const char *argv0)
{
    printf("Usage:\n");
    printf("  %s            start PC\n", argv0);
    printf("Options:\n");
    printf("  -c, --config-file=<path>     path to config file\n");
    printf("  -o, --number-options=<path>  number of options to parse from config file\n");
    printf("  -h, --help>                  this help\n");

}

int main(int argc, char *argv[])
{
  int ret;
  char *configpath = "fvmconfig"; //hard-coded default values
  int optionsParse = 4;
  fvmAllocHandle_t handle;

  while (1) {
    int c;

    static struct option long_options[] = {
      { "config-file", 1, NULL, 'c' },
      { "number-options", 1, NULL, 'o' },
      { "help", 0, NULL, 'h' },
      { 0 }
    };

    c = getopt_long(argc, argv, "c:o:h", long_options, NULL);
    if (c == -1)
      break;

    switch (c) {
    case 'c':     
      configpath = optarg;
      break;
    case 'o':     
      optionsParse = strtol(optarg, NULL, 0);
      break;
    case 'h':
      usage(argv[0]);
      return 1;
    default:
      break;
    }
  }

  //create a module loader
  sdpa::modules::ModuleLoader::ptr_t loader = sdpa::modules::ModuleLoader::create();
		

  // load module to work with fvm
  try
    {                         
      loader->load("pc-mod", "/u/herc/machado/current_work/sdpa/fvm_pc_separated/lib/libpc-mod.so");
    }
  catch (sdpa::modules::ModuleLoadFailed &mlf)
    {
      std::cout << "An exception occurred loading pc-mod " << endl;
    
    }

  sdpa::modules::Module  &mod = loader->get("pc-mod");
  sdpa::modules::Module::data_t params;

  params["path"] = sdpa::wf::Parameter("path", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(configpath));
  params["numoptions"] = sdpa::wf::Parameter("numoptions", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(optionsParse));

  try 
    {
      mod.call("readConfigFile", params);
    }
  catch (const std::exception & ex) {
    std::cout << "exception calling func" << endl;
  }

  /*  with type checking enabled, it throws exception */
  configFile_t *config = (configFile_t *)params["out"].token().data_as<void*>();

  printf("%s\n",config->msqfile);


  params["configFile"] = sdpa::wf::Parameter("configFile", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(config));

  try 
    {
      mod.call("fvmConnect", params);
    }
  catch (const std::exception & ex) {
    std::cout << "exception calling connect func" << endl;
  }

  ret = params["out"].token().data_as<int>();
  if(ret == -1)
    {
      printf("Couldn't connect to FVM. Is there one running?\n");
      return -1;
    }

  /* Start loading tests modules */
  try
    {
      loader->load("testDynamicArrayInit-mod", "/u/herc/machado/current_work/sdpa/fvm_pc_separated/tests/testDynamicArrayInit-mod.so");
    }
  catch (sdpa::modules::ModuleLoadFailed &mlf)
    {
      std::cout << "An exception occurred while loading module" << endl;
    }

  sdpa::modules::Module  &mod1 = loader->get("testDynamicArrayInit-mod");

  sdpa::modules::Module::data_t params1;
  try 
    {
      mod1.call("test_dynamicArrayInit", params1);
    }
  catch (const std::exception & ex) {
	  std::cout << "exception calling test dynamic array init" << endl;
  }

  handle  = params1["out"].token().data_as<fvmAllocHandle_t>();
  printf("Handle to use is %lu\n", handle);


  // Leave...

  try 
    {
      mod.call("fvmLeave", params);
    }
  catch (const std::exception & ex) {
    std::cout << "exception calling leave func" << endl;
  }


  return 0;
}

