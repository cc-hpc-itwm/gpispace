#include <sdpa/modules/Module.hpp>
#include <string>

using namespace sdpa::modules;

// module function implementations
void HelloWorld(const sdpa::modules::Module::input_data_t &in, sdpa::modules::Module::output_data_t &out) {
  out.push_back(sdpa::daemon::Token(std::string("hello world")));
}
void DoNothing(const sdpa::modules::Module::input_data_t &in, sdpa::modules::Module::output_data_t &out) {
}

extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, HelloWorld);
    SDPA_REGISTER_FUN(mod, DoNothing);
  }
}
