#include <gspc/task/Result.hpp>

#include <util-generic/print_container.hpp>

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
                     ( "{", ", ", "}", success.outputs
                     , [&] (auto& s, auto const& x) -> decltype (s)
                       {
                         return s << x.first << " -> " << x.second;
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
