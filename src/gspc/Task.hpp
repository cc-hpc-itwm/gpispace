#pragma once

#include <gspc/resource/Class.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Implementation.hpp>

#include <boost/variant.hpp>

#include <ostream>
#include <utility>
#include <vector>

namespace gspc
{
  namespace task
  {
    using Input = std::vector<char>;
  }

  //! does a generic task exist?
  struct Task
  {
    task::Input input;

    using SingleResource = std::pair<resource::Class, task::Implementation>;
    using SingleResourceWithPreference = std::vector<SingleResource>;
    using SingleResourceWithCost = std::vector<std::pair<SingleResource, double>>;
    using CoallocationSingleClass = std::pair<SingleResource, std::size_t>;
    using CoallocationSingleClassWithPreference =
      std::vector<CoallocationSingleClass>;

    using Requirements = boost::variant
      < SingleResource                        // "socket"
      , SingleResourceWithPreference          // ["gpu", "cpu"]
      , SingleResourceWithCost                // [("cpu", 10.0), ("gpu", 3.0)]
      , CoallocationSingleClass               // ("gpu", 4)
      , CoallocationSingleClassWithPreference // either ("gpu", 4) or ("cpu", 2)
      >;

    Requirements requirements;

    //! *relative to a single task*, static, produced by workflow engine
    //! \todo everything also with memory

    friend std::ostream& operator<< (std::ostream&, Task const&);

    template<typename Archive>
      void serialize (Archive& ar, unsigned int);
  };
}

#include <gspc/Task.ipp>
