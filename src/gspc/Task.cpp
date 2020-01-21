#include <gspc/Task.hpp>

#include <util-generic/print_container.hpp>

#include <boost/io/ios_state.hpp>

#include <iomanip>

namespace gspc
{
  std::ostream& operator<< (std::ostream& os, Task const& task)
  {
    return os << "Task {"
              << "id = " << task.id
              << ", resource_class = " << task.resource_class
              << ", input = "
              << fhg::util::print_container
                 ( "hex[", " ", "]", task.input
                 , [&] (auto& s, auto const& x) -> decltype (s)
                   {
                     boost::io::ios_all_saver const save (s);

                     return s << std::hex
                              << std::setw (2)
                              << std::setfill ('0')
                              << (static_cast<std::size_t> (x) & 0xFF);
                   }
                 )
              << ", so = " << task.so
              << ", symbol = " << task.symbol
              << "}"
      ;
  }
}
