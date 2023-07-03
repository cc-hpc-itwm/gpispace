// Copyright (C) 2023 Fraunhofer ITWM
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
     template<typename T> T const& get (Key) const;
     template<typename T> std::vector<T> get_all (Key, std::size_t) const;

  private:
     void assert_key_count
      ( Key key
      , std::size_t expected_count
      ) const;

     template<typename T, typename TypeDescription>
       T const& get_impl (Key, TypeDescription) const;
     template<typename T, typename TypeDescription>
       std::vector<std::string> get_all_impl
         (Key, TypeDescription, std::size_t) const;
  };

  template<>
    we::type::literal::control const& WorkflowResult::get (Key) const;
  template<>
    std::vector<std::string> WorkflowResult::get_all (Key, std::size_t) const;
}
