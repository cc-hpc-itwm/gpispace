// Copyright (C) 2019-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <unordered_map>

#include <gspc/xml/parse/type/mod.hpp>
#include <gspc/xml/parse/type/preferences.hpp>
#include <optional>



    namespace gspc::xml::parse::type
    {
      using multi_module_map = std::unordered_map < preference_type
                                                  , module_type
                                                  >;

      struct multi_module_type
      {
      public:
        multi_module_type () = default;

        void add (module_type const& mod);

        multi_module_map const& modules() const;

        std::optional<::gspc::we::type::eureka_id_type> const& eureka_id() const;

      private:
        multi_module_map  _modules;
      };

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream&, multi_module_type const&);
      }
    }
