// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#pragma once

#include <aggregate_sum/ValuesOnPorts.hpp>

#include <cstddef>

namespace aggregate_sum
{
  class WorkflowResult : public ValuesOnPorts
  {
  public:
     using ValuesOnPorts::ValuesOnPorts;

     // asserts there is exactly one occurence of key
     template<typename T> T const& get (Key key) const;

  private:
     void assert_key_count
      ( Key key
      , std::size_t expected_count
      ) const;

     template<typename T, typename TypeDescription>
       T const& get_impl (Key key, TypeDescription type_description) const;
  };

  template<> int const& WorkflowResult::get (Key key) const;
}
