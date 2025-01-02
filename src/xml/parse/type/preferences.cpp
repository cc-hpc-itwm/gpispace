// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/type/preferences.hpp>

#include <xml/parse/error.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      preferences_type::preferences_type
        (std::list<preference_type> targets)
        : _targets (std::move (targets))
      {}

      std::list<preference_type> const&
        preferences_type::targets() const
      {
        return _targets;
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream& s
                  , preferences_type const& cs
                  )
        {
          s.open ("preferences");

          for (auto const& pref_type : cs.targets())
          {
            s.open ("target");
            s.content (pref_type);
            s.close ();
          }

          s.close ();
        }
      }
    }
  }
}
