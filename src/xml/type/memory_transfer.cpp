// Copyright (C) 2014,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/type/memory_transfer.hpp>
#include <gspc/xml/parse/util/position.hpp>

#include <gspc/util/xml.hpp>
#include <optional>



    namespace gspc::xml::parse::type
    {
      memory_transfer_type::memory_transfer_type
        ( util::position_type const& position_of_definition
        , std::string const& global
        , std::string const& local
        , ::gspc::we::type::property::type const& properties
        , std::optional<bool> const& allow_empty_ranges
        )
          : with_position_of_definition (position_of_definition)
          , _global (global)
          , _local (local)
          , _properties (properties)
          , _allow_empty_ranges (allow_empty_ranges)
      {}

      std::optional<bool> const&
        memory_transfer_type::allow_empty_ranges() const
      {
        return _allow_empty_ranges;
      }

      memory_get::memory_get
        ( util::position_type const& position_of_definition
        , std::string const& global
        , std::string const& local
        , ::gspc::we::type::property::type const& properties
        , std::optional<bool> const& allow_empty_ranges
        )
          : memory_transfer_type
              ( position_of_definition
              , global
              , local
              , properties
              , allow_empty_ranges
              )
      {}

      memory_put::memory_put
        ( util::position_type const& position_of_definition
        , std::string const& global
        , std::string const& local
        , ::gspc::we::type::property::type const& properties
        , std::optional<bool> const& not_modified_in_module_call
        , std::optional<bool> const& allow_empty_ranges
        )
          : memory_transfer_type
              ( position_of_definition
              , global
              , local
              , properties
              , allow_empty_ranges
              )
          , _not_modified_in_module_call (not_modified_in_module_call)
      {}

      memory_getput::memory_getput
        ( util::position_type const& position_of_definition
        , std::string const& global
        , std::string const& local
        , ::gspc::we::type::property::type const& properties
        , std::optional<bool> const& not_modified_in_module_call
        , std::optional<bool> const& allow_empty_ranges
        )
          : memory_transfer_type
              ( position_of_definition
              , global
              , local
              , properties
              , allow_empty_ranges
              )
          , _not_modified_in_module_call (not_modified_in_module_call)
      {}

      namespace dump
      {
        namespace
        {
          void dump_transfer ( ::gspc::util::xml::xmlstream& s
                             , memory_transfer_type const& mt
                             )
          {
            s.attr ("allow-empty-ranges", mt.allow_empty_ranges());
            s.open ("global");
            s.content (mt.global());
            s.close();
            s.open ("local");
            s.content (mt.local());
            s.close();
          }
        }

        void dump (::gspc::util::xml::xmlstream& s, memory_get const& mg)
        {
          s.open ("memory-get");
          dump_transfer (s, mg);
          s.close();
        }
        void dump (::gspc::util::xml::xmlstream& s, memory_put const& mp)
        {
          s.open ("memory-put");
          s.attr
            ("not-modified-in-module-call", mp.not_modified_in_module_call());
          dump_transfer (s, mp);
          s.close();
        }
        void dump (::gspc::util::xml::xmlstream& s, memory_getput const& mp)
        {
          s.open ("memory-getput");
          s.attr
            ("not-modified-in-module-call", mp.not_modified_in_module_call());
          dump_transfer (s, mp);
          s.close();
        }
      }
    }
