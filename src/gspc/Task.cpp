#include <gspc/Task.hpp>

#include <util-generic/print_container.hpp>

namespace gspc
{
  std::ostream& operator<< (std::ostream& os, Task const& task)
  {
    return os << "Task {"
              << "id = " << task.id
              << ", resource_class = " << task.resource_class
              << ", inputs = "
              << fhg::util::print_container
                 ( "{", ", ", "}", task.inputs
                 , [&] (auto& s, auto const& x) -> decltype (s)
                   {
                     return s << x.first << " -> " << x.second;
                   }
                 )
              << ", so = " << task.so
              << ", symbol = " << task.symbol
              << "}"
      ;
  }
}
