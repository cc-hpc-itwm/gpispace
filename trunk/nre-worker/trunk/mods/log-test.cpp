#include <sdpa/modules/Macros.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free

#include <fhglog/fhglog.hpp>

using namespace sdpa::modules;

static void RunTest() throw (std::exception)
{
  MLOG(INFO, "performing RunTest()...");
}

SDPA_MOD_INIT_START(log-test)
{
  // run test cases
  RunTest();
}
SDPA_MOD_INIT_END(log-test)
