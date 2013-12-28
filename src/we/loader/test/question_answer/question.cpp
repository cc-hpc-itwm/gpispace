#include <we/loader/IModule.hpp>
#include <we/loader/macros.hpp>

#include "answer.hpp"

static void question ( gspc::drts::context*
                     , const expr::eval::context&
                     , expr::eval::context& output
                     )
{
  output.bind ("out", get_answer());
  output.bind ("ans", the_answer);
  get_answer();
}


WE_MOD_INITIALIZE_START (question);
{
  get_answer();
  WE_REGISTER_FUN (question);
}
WE_MOD_INITIALIZE_END (question);

WE_MOD_FINALIZE_START (question);
{
}
WE_MOD_FINALIZE_END (question);
