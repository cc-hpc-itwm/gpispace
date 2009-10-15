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

// add two integer parameters and store the result in "out"
void Add(sdpa::modules::Module::data_t &params)
{
  const long long a = params["a"].token().data_as<long long>();
  const long long b = params["b"].token().data_as<long long>();

  params["out"].token().data(a+b);
}

extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, HelloWorld);
    SDPA_REGISTER_FUN(mod, DoNothing);
    SDPA_REGISTER_FUN(mod, Add);
  }
}

