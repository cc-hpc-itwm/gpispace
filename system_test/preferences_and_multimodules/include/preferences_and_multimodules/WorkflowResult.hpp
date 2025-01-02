// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <preferences_and_multimodules/ValuesOnPorts.hpp>

#include <we/type/value.hpp>

#include <cstddef>
#include <string>
#include <vector>

namespace preferences_and_multimodules
{
  class WorkflowResult : public ValuesOnPorts
  {
  public:
     using ValuesOnPorts::ValuesOnPorts;

     // asserts there is exactly one occurence of key
     template<typename T> T const& at (Key) const;
     template<typename T> std::vector<T> at_all (Key, std::size_t) const;

  private:
     void assert_key_count
      ( Key key
      , std::size_t expected_count
      ) const;

     template<typename T, typename TypeDescription>
       T const& at_implementation (Key, TypeDescription) const;
     template<typename T, typename TypeDescription>
       std::vector<std::string> at_all_implementation
         (Key, TypeDescription, std::size_t) const;
  };

  template<>
    we::type::literal::control const& WorkflowResult::at (Key) const;
  template<>
    std::vector<std::string> WorkflowResult::at_all (Key, std::size_t) const;
}
