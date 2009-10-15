#include <sdpa/modules/Module.hpp>
#include <string>

using namespace sdpa::modules;

// module function implementations
void HelloWorld(sdpa::modules::Module::data_t &params)
{
  params["out"].token().data("hello world");
}

void DoNothing(sdpa::modules::Module::data_t &)
{
}

extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, HelloWorld);
    SDPA_REGISTER_FUN(mod, DoNothing);
  }
}
