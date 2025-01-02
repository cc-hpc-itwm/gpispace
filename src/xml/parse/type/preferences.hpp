// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fhg/util/xml.fwd.hpp>

#include <list>
#include <string>
#include <unordered_map>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      using preference_type = std::string;

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
                  , preferences_type const& cs
                  );
      }
    }
  }
}
