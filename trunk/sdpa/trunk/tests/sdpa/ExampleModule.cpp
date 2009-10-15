#include <sdpa/modules/Module.hpp>
#include <string>
#include <cstdlib> // malloc, free

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

void Malloc(sdpa::modules::Module::data_t &p) throw (std::exception)
{
  const std::size_t bytes = p["size"].token().data_as<std::size_t>();
  void *ptr = std::malloc(bytes);
  if (0 == ptr) {
    throw std::runtime_error("memory allocation failed");
  }
  p["out"].token().data(ptr);
}

void Free(sdpa::modules::Module::data_t &p) throw (std::exception)
{
  void *ptr = p["ptr"].token().data_as<void*>();
  if (ptr) {
    free(ptr);
    p["ptr"].token().data((void*)0);
  } else {
    throw std::runtime_error("double free detected");
  }
}

extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, HelloWorld);
    SDPA_REGISTER_FUN(mod, DoNothing);
    SDPA_REGISTER_FUN(mod, Add);
    SDPA_REGISTER_FUN(mod, Malloc);
    SDPA_REGISTER_FUN(mod, Free);
  }
}

