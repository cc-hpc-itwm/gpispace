// dipti.shankar@itwm.fraunhofer.de

#pragma once

#include <fhg/util/xml.fwd.hpp>

#include <string>
#include <list>
#include <unordered_map>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::string preference_type;

      struct preferences_type
      {
      private:
        std::list<type::preference_type> _list;

      public:
        preferences_type();

        void add_unique_target_in_order
          (const preference_type& target_name);
        bool empty() const;

        const std::list<type::preference_type>&
          get_preference_list() const;
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const preferences_type & cs
                  );
      }
    }
  }
}
