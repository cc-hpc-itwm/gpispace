#include <sdpa/modules/Module.hpp>

using namespace sdpa::modules;

void prefsum (Module::data_t &params)
{
  // at the moment: do nothing
}

SDPA_MOD_INIT_START(prefsum)
{
  SDPA_REGISTER_FUN(prefsum);
}
SDPA_MOD_INIT_END(prefsum)
