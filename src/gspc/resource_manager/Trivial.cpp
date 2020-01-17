#include <gspc/resource_manager/Trivial.hpp>

namespace gspc
{
  namespace resource_manager
  {
    Trivial::Acquired Trivial::acquire (resource::Class resource_class)
    {
      return WithPreferences::acquire ({resource_class});
    }
  }
}
