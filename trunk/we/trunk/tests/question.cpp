#include <we/loader/IModule.hpp>
#include <we/loader/macros.hpp>

#include "answer.hpp"

WE_MOD_INITIALIZE_START (question);
{
  std::cerr << "The answer to life, the universe and everything := " << get_answer() << std::endl;
  std::cerr << "The answer to life, the universe and everything := " << the_answer << std::endl;
}
WE_MOD_INITIALIZE_END (answer);

WE_MOD_FINALIZE_START (answer);
{
}
WE_MOD_FINALIZE_END (answer);
