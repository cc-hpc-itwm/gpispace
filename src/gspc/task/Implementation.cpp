#include <gspc/task/Implementation.hpp>

namespace gspc
{
  namespace task
  {
    std::ostream& operator<< ( std::ostream& os
                             , Implementation const& implementation
                             )
    {
      return os << "Implementation {" << implementation.so
                << ", " << implementation.symbol
                << "}"
        ;
    }
  }
}
