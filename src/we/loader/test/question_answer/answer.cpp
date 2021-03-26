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
