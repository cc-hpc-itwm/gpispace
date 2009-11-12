#include <sdpa/modules/Module.hpp>

using namespace sdpa::modules;

void finalize (Module::data_t &params)
{
  // at the moment: do nothing
}

SDPA_MOD_INIT_START(finalize)
{
  SDPA_REGISTER_FUN(finalize);
}
SDPA_MOD_INIT_END(finalize)
