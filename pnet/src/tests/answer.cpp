#include <we/loader/IModule.hpp>
#include <we/loader/macros.hpp>

#include "answer.hpp"

int the_answer;
int get_answer ()
{
  static int ans = 42;
  return ++ans;
}

static void answer ( gspc::drts::context *
                   , const expr::eval::context& input
                   , expr::eval::context& output
                   )
{
  output.bind ("out", pnet::type::value::value_type (42L));
}

WE_MOD_INITIALIZE_START (answer);
{
  the_answer = 42;
  WE_REGISTER_FUN (answer);
}
WE_MOD_INITIALIZE_END (answer);

WE_MOD_FINALIZE_START (answer);
{
}
WE_MOD_FINALIZE_END (answer);
