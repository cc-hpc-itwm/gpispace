#include <we/loader/macros.hpp>

#include <we/loader/test/question_answer/answer.hpp>

static void question ( drts::worker::context*
                     , const expr::eval::context&
                     , expr::eval::context& output
                     , std::map<std::string, void*> const&
                     )
{
  output.bind_and_discard_ref ("out", get_answer());
  output.bind_and_discard_ref ("ans", the_answer);
  get_answer();
}


WE_MOD_INITIALIZE_START()
{
  get_answer();
  WE_REGISTER_FUN_AS (question, "question");
}
WE_MOD_INITIALIZE_END()
