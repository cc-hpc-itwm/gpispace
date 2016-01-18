// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <list>
#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct conditions_type : public std::list<std::string>
      {
        std::string flatten() const;
      };

      conditions_type operator+ (conditions_type, const conditions_type&);

      struct function_type;
    }
  }
}
