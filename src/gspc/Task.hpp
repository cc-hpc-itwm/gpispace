#pragma once

#include <gspc/heureka/Group.hpp>
#include <gspc/resource/Class.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/value_type.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <ostream>
#include <string>
#include <unordered_map>

namespace gspc
{
  struct Task
  {
    task::ID id;

    resource::Class resource_class;
    boost::optional<heureka::Group> heureka_group;
    //! \todo why not std::vector<char>?
    using Inputs = std::unordered_multimap<std::string, value_type>;
    Inputs inputs;
    boost::filesystem::path so;
    using Symbol = std::string;
    Symbol symbol;

    friend std::ostream& operator<< (std::ostream&, Task const&);

    template<typename Archive>
      void serialize (Archive& ar, unsigned int);
  };
}

#include <gspc/Task.ipp>
