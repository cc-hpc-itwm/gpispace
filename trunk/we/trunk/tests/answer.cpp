#include <we/loader/IModule.hpp>
#include <we/loader/macros.hpp>

#include "answer.hpp"

int the_answer;
int get_answer ()
{
  static int ans = 42;
  return ++ans;
}

static void answer (void * state, const we::loader::input_t & input, we::loader::output_t & output)
{
  std::cerr << "state := " << state << std::endl;
  std::cerr << "input := " << input << std::endl;
  we::loader::put_output (output, "out", 42L);
}

WE_MOD_INITIALIZE_START (answer);
{
  mod->state((void*)(0x42));
  the_answer = 42;
  WE_REGISTER_FUN (answer);
}
WE_MOD_INITIALIZE_END (answer);

WE_MOD_FINALIZE_START (answer);
{
}
WE_MOD_FINALIZE_END (answer);
