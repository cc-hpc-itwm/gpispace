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


// test that uses the handle returned from the Init test
// the handle is simply passed via command line option -d
// the idea is to have other process container started at other nodes with that handle
// and should be able to see the right data of the array (stored on the global fvm space)

static void usage(const char *argv0)
{
    printf("Usage:\n");
    printf("  %s            start PC\n", argv0);
    printf("Options:\n");
    printf("  -c, --config-file=<path>      path to config file\n");
    printf("  -o, --number-options=<path>   number of options to parse from config file\n");
    printf("  -d, --dynamic-array=<handle>  handle of dynamic array to be used number\n");
    printf("  -h, --help>                   this help\n");

}

int main(int argc, char *argv[])
{
  int ret;
  char *configpath = "fvmconfig"; //hard-coded default values
  int optionsParse = 4;
  fvmAllocHandle_t handle=0;

  while (1) {
    int c;

    static struct option long_options[] = {
      { "config-file", 1, NULL, 'c' },
      { "number-options", 1, NULL, 'o' },
      { "handle", 1, NULL, 'd'},
      { "help", 0, NULL, 'h' },
      { 0 }
    };

    c = getopt_long(argc, argv, "c:o:d:h", long_options, NULL);
    if (c == -1)
      break;

    switch (c) {
    case 'c':     
      configpath = optarg;
      break;
    case 'o':     
      optionsParse = strtol(optarg, NULL, 0);
      break;
    case 'd':
      handle =  strtol(optarg, NULL, 0);
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
  params["handle"] = sdpa::wf::Parameter("handle", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(handle));

  try
    {
      loader->load("testDynamicArrayUsage-mod", "/u/herc/machado/current_work/sdpa/fvm_pc_separated/tests/testDynamicArrayUsage-mod.so");
    }
  catch (sdpa::modules::ModuleLoadFailed &mlf)
    {
      std::cout << "An exception occurred while loading module" << endl;
    }

  sdpa::modules::Module  &mod1 = loader->get("testDynamicArrayUsage-mod");

  sdpa::modules::Module::data_t params1;
  params1["handle"] = sdpa::wf::Parameter("handle", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(handle));
  try 
    {
      mod1.call("test_dynamicArrayUsage", params1);
    }
  catch (const std::exception & ex) {
	  std::cout << "exception calling test dynamic array usage" << endl;
  }


  try 
    {
      mod.call("fvmLeave", params);
    }
  catch (const std::exception & ex) {
    std::cout << "exception calling leave func" << endl;
  }


  return 0;
}

