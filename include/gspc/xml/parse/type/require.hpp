// Copyright (C) 2011-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/xml.fwd.hpp>

#include <string>
#include <unordered_set>



    namespace gspc::xml::parse::type
    {
      using require_key_type = std::string;

      struct requirements_type
      {
      private:
        using set_type = std::unordered_set<require_key_type>;

      public:
        requirements_type() = default;

        void set (require_key_type const& key);

        using const_iterator = set_type::const_iterator;

        const_iterator begin () const;
        const_iterator end () const;

        void join (requirements_type const& reqs);

      private:
        set_type _set;
      };

      namespace dump
      {
        void dump ( ::gspc::util::xml::xmlstream & s
                  , requirements_type const& cs
                  );
      }
    }
