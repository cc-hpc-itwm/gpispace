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
      //! \todo why not std::vector<char>
      using Outputs = std::unordered_multimap<std::string, value_type>;
      Outputs outputs;

      friend std::ostream& operator<< (std::ostream&, Result const&);

      template<typename Archive>
        void serialize (Archive& ar, unsigned int);
    };
  }
}

#include <gspc/task/Result.ipp>
