#include <sdpa/modules/Module.hpp>

using namespace sdpa::modules;

void writeoutp (Module::data_t &params)
{
  // at the moment: do nothing
}

SDPA_MOD_INIT_START(writeoutp)
{
  SDPA_REGISTER_FUN(writeoutp);
}
SDPA_MOD_INIT_END(writeoutp)
