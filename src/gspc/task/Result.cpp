#include <gspc/task/Result.hpp>

#include <util-generic/print_container.hpp>

#include <boost/io/ios_state.hpp>

#include <iomanip>

namespace gspc
{
  namespace task
  {
    namespace result
    {
      std::ostream& operator<< (std::ostream& os, Success const& success)
      {
        return os << "Success: outputs: "
                  << fhg::util::print_container
                     ( "hex[", " ", "]", success.output
                     , [&] (auto& s, auto const& x) -> decltype (s)
                       {
                         boost::io::ios_all_saver const save (s);

                         return s << std::hex
                                  << std::setw (2)
                                  << std::setfill ('0')
                                  << (static_cast<std::size_t> (x) & 0xFF);
                       }
                     )
                     ;
      }
      std::ostream& operator<< (std::ostream& os, Failure const& failure)
      {
        return os << "Failure: exception: "
                  << fhg::util::exception_printer (failure.exception)
          ;
      }
      std::ostream& operator<< (std::ostream& os, CancelIgnored const& ci)
      {
        return os << "CancelIgnored: after_inject: " << ci.after_inject;
      }
      std::ostream& operator<< (std::ostream& os, CancelOptional const& co)
      {
        return os << "CancelOptional: after_inject: " << co.after_inject;
      }
      std::ostream& operator<< (std::ostream& os, Postponed const& pp)
      {
        return os << "Postponed: reason: " << pp.reason;
      }
    }
  }
}
