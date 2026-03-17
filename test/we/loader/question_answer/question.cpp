// Copyright (C) 2010,2013-2016,2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/loader/macros.hpp>

#include <test/we/loader/question_answer/answer.hpp>

static void question ( drts::worker::context*
                     , gspc::we::expr::eval::context const&
                     , gspc::we::expr::eval::context& output
                     , std::map<std::string, void*> const&
                     )
{
  output.bind_and_discard_ref ({"out"}, get_answer());
  output.bind_and_discard_ref ({"ans"}, the_answer);
  get_answer();
}


WE_MOD_INITIALIZE_START()
{
  get_answer();
  WE_REGISTER_FUN_AS (question, "question");
}
WE_MOD_INITIALIZE_END()
