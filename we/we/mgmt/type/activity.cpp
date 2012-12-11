// {petry,rahn}@itwm.fhg.de

#include <we/mgmt/type/activity.hpp>

#include <iostream>

namespace we
{
  namespace mgmt
  {
    namespace type
    {
      bool operator== (const activity_t& a, const activity_t& b)
      {
        return a.id() == b.id();
      }

      std::ostream& operator<< (std::ostream &os, const activity_t & act)
      {
        act.writeTo (os);
        return os;
      }
    }
  }
}
