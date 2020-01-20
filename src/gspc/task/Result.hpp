#pragma once

#include <gspc/value_type.hpp>
#include <gspc/heureka/Group.hpp>

#include <boost/optional.hpp>

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

      //! \todo any group? only the group the task is a member? multiple?
      boost::optional<heureka::Group> heureka_group;

      friend std::ostream& operator<< (std::ostream&, Result const&);

      template<typename Archive>
        void serialize (Archive& ar, unsigned int);
    };
  }
}

#include <gspc/task/Result.ipp>
