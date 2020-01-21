#pragma once

#include <gspc/resource/Class.hpp>
#include <gspc/task/ID.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <ostream>
#include <string>
#include <vector>

namespace gspc
{
  struct Task
  {
    task::ID id;

    resource::Class resource_class;
    using Input = std::vector<char>;
    Input input;
    boost::filesystem::path so;
    using Symbol = std::string;
    Symbol symbol;

    friend std::ostream& operator<< (std::ostream&, Task const&);

    template<typename Archive>
      void serialize (Archive& ar, unsigned int);
  };
}

#include <gspc/Task.ipp>
