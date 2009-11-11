#include <assert.h>
#include <Module.hpp>
#include <pc.h>

using namespace sdpa::modules;
using namespace std;


//test local memory
void test_api(Module::data_t &params)
{
  printf("*******************************************************************************\n");
  printf("******************************* Test API **************************************\n");
  printf("*******************************************************************************\n");

  printf("My rank is %d\n", fvmGetRank());
  printf("Total node count: %d\n", fvmGetNodeCount());


}



// The init function will call all tests implemented
extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, test_api);

    //actually call the functions
    sdpa::modules::Module::data_t params;
    test_api(params);

  }
}

