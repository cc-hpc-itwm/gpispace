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
