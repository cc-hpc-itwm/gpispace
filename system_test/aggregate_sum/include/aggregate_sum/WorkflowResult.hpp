// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
     template<typename T> T const& at (Key key) const;

  private:
     void assert_key_count
      ( Key key
      , std::size_t expected_count
      ) const;

     template<typename T, typename TypeDescription>
       T const& at_implementation (Key, TypeDescription) const;
  };

  template<> int const& WorkflowResult::at (Key key) const;
}
