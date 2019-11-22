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

      class preferences_type
      {
        private:
          std::list<preference_type> _targets;

        public:
          preferences_type () = default;

          //! \note assumes targets is a list of unique names
          //! \note insertion order reflects preference ordering
          preferences_type (std::list<preference_type> targets);

          std::list<preference_type> const& targets() const;
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream& s
                  , const preferences_type& cs
                  );
      }
    }
  }
}
