#include <sdpa/modules/Macros.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free

using namespace sdpa::modules;

// module function implementations
void HelloWorld(data_t &params)
{
  params["out"].token().data("hello world");
}

void DoNothing(data_t &)
{

}

// add two integer parameters and store the result in "out"
void Add(data_t &params)
{
  const long a = params.at("a").token().data_as<long>();
  const long b = params.at("b").token().data_as<long>();

  params["out"].token().data(a+b);
}

void Malloc(data_t &p) throw (std::exception)
{
  const std::size_t bytes = p["size"].token().data_as<std::size_t>();
  void *ptr = std::malloc(bytes);
  if (0 == ptr) {
    throw std::runtime_error("memory allocation failed");
  }
  p["out"].token().data(ptr);
}

void Free(data_t &p) throw (std::exception)
{
  void *ptr = p["ptr"].token().data_as<void*>();
  if (ptr) {
    free(ptr);
    p["ptr"].token().data((void*)0);
  } else {
    throw std::runtime_error("double free detected");
  }
}

void Update(data_t &p) throw (std::exception)
{
  int *ptr = (int*)p["ptr"].token().data_as<void*>();
  unsigned int value= p["value"].token().data_as<unsigned int>();
  *ptr = value;
}

static void RunTest() throw (std::exception)
{
  std::cout << "running RunTest example" << std::endl;
}

SDPA_MOD_INIT_START(example-mod)
{
  SDPA_REGISTER_FUN(HelloWorld);
  SDPA_REGISTER_FUN(DoNothing);
  SDPA_REGISTER_FUN(Add);
  SDPA_REGISTER_FUN(Malloc);
  SDPA_REGISTER_FUN(Free);
  SDPA_REGISTER_FUN(Update);

  // run test cases
  RunTest();
}
SDPA_MOD_INIT_END(example-mod)
