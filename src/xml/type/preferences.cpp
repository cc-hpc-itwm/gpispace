// Copyright (C) 2019,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/preferences.hpp>

#include <gspc/xml/parse/error.hpp>

#include <gspc/util/xml.hpp>



    namespace gspc::xml::parse::type
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
        void dump ( ::gspc::util::xml::xmlstream& s
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
