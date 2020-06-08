#pragma once

#include <iosfwd>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      class complete
      {
      public:
        complete (const std::string&);
        const std::string& tname() const;
      private:
        const std::string& _tname;
      };
      std::ostream& operator<< (std::ostream&, const complete&);
    }
  }
}
