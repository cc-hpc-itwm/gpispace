#pragma once

#include <gspc/resource/Class.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/value_type.hpp>

#include <boost/filesystem/path.hpp>

#include <ostream>
#include <string>
#include <unordered_map>

namespace gspc
{
  struct Task
  {
    task::ID id;

    resource::Class resource_class;
    //! \todo why not std::vector<char>?
    std::unordered_map<std::string, value_type> inputs;
    boost::filesystem::path so;
    std::string symbol;

    friend std::ostream& operator<< (std::ostream&, Task const&);

    template<typename Archive>
      void serialize (Archive& ar, unsigned int);
  };
}

#include <gspc/Task.ipp>
