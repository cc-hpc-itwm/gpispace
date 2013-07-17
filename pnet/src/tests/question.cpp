#include <we/loader/IModule.hpp>
#include <we/loader/macros.hpp>

#include "answer.hpp"

static void question (void *, const we::loader::input_t & input, we::loader::output_t & output)
{
  std::cerr << "input := " << input << std::endl;
  output. bind ("out", value::type ((long)get_answer()));
  std::cerr << "answer := " << get_answer() << std::endl;
}


WE_MOD_INITIALIZE_START (question);
{
  std::cerr << "The answer to life, the universe and everything := " << get_answer() << std::endl;
  std::cerr << "The answer to life, the universe and everything := " << the_answer << std::endl;
  WE_REGISTER_FUN (question);
}
WE_MOD_INITIALIZE_END (question);

WE_MOD_FINALIZE_START (question);
{
}
WE_MOD_FINALIZE_END (question);
