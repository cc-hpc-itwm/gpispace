// Copyright (C) 2019,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/xml.fwd.hpp>

#include <list>
#include <string>
#include <unordered_map>



    namespace gspc::xml::parse::type
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
        void dump ( ::gspc::util::xml::xmlstream& s
                  , preferences_type const& cs
                  );
      }
    }
