#include <gspc/task/Result.hpp>

#include <util-generic/print_container.hpp>

namespace gspc
{
  namespace task
  {
    std::ostream& operator<< (std::ostream& os, Result const& result)
    {
      return os << "Result:"
        << fhg::util::print_container
           ( "  ", "\n  ", "\n", result.outputs
           , [&] (auto& s, auto const& x) -> decltype (s)
             {
               return s << x.first << " -> " << x.second;
             }
           )
        ;
    }
  }
}
