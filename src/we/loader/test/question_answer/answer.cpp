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
                   , std::map<std::string, void*> const&
                   )
{
  output.bind_and_discard_ref ({"out"}, 42L);
}

WE_MOD_INITIALIZE_START()
{
  the_answer = 42L;
  WE_REGISTER_FUN_AS (answer,  "answer");
}
WE_MOD_INITIALIZE_END()
