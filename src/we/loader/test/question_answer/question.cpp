// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <we/loader/macros.hpp>

#include <we/loader/test/question_answer/answer.hpp>

static void question ( drts::worker::context*
                     , expr::eval::context const&
                     , expr::eval::context& output
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
