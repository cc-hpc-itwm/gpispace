#pragma once

#include <gspc/value_type.hpp>

#include <ostream>
#include <string>
#include <unordered_map>

namespace gspc
{
  namespace task
  {
    struct Result
    {
      std::unordered_map<std::string, value_type> outputs;

      friend std::ostream& operator<< (std::ostream&, Result const&);

      template<typename Archive>
        void serialize (Archive& ar, unsigned int);
    };
  }
}

#include <gspc/task/Result.ipp>
