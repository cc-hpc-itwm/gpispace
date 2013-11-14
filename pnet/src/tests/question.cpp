#include <we/loader/IModule.hpp>
#include <we/loader/macros.hpp>
#include <iostream>

#include "answer.hpp"

static void question (gspc::drts::context *, const expr::eval::context & input, expr::eval::context& output)
{
  std::cerr << "input := " << input << std::endl;
  output. bind ("out", pnet::type::value::value_type ((long)get_answer()));
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
