#include <we/loader/test/question_answer/answer.hpp>

#include <we/loader/macros.hpp>

long the_answer;
long get_answer ()
{
  static long ans (42);
  return ++ans;
}

static void answer ( drts::worker::context*
                   , const expr::eval::context&
                   , expr::eval::context& output
                   )
{
  output.bind ("out", 42L);
}

WE_MOD_INITIALIZE_START (answer);
{
  the_answer = 42L;
  WE_REGISTER_FUN (answer);
}
WE_MOD_INITIALIZE_END (answer);

WE_MOD_FINALIZE_START (answer);
{
}
WE_MOD_FINALIZE_END (answer);
