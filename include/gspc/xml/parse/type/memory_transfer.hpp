// Copyright (C) 2014-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/xml/parse/type/with_position_of_definition.hpp>

#include <gspc/xml/parse/util/position.fwd.hpp>

#include <gspc/we/type/property.hpp>

#include <gspc/util/xml.fwd.hpp>


#include <string>
#include <optional>



    namespace gspc::xml::parse::type
    {
      struct memory_transfer_type : with_position_of_definition
      {
      public:
        memory_transfer_type ( util::position_type const&
                             , std::string const& global
                             , std::string const& local
                             , ::gspc::we::type::property::type const&
                             , std::optional<bool> const&
                             );

        std::string const& global() const
        {
          return _global;
        }
        std::string const& local() const
        {
          return _local;
        }
        ::gspc::we::type::property::type const& properties() const
        {
          return _properties;
        }
        std::optional<bool> const& allow_empty_ranges() const;

      private:
        std::string _global;
        std::string _local;
        ::gspc::we::type::property::type _properties;
        std::optional<bool> _allow_empty_ranges;
      };

      struct memory_get : memory_transfer_type
      {
      public:
        memory_get ( util::position_type const&
                   , std::string const& global
                   , std::string const& local
                   , ::gspc::we::type::property::type const&
                   , std::optional<bool> const&
                   );
      };

      struct memory_put : memory_transfer_type
      {
      public:
        memory_put ( util::position_type const&
                   , std::string const& global
                   , std::string const& local
                   , ::gspc::we::type::property::type const&
                   , std::optional<bool> const&
                   , std::optional<bool> const&
                   );
        std::optional<bool> const& not_modified_in_module_call() const
        {
          return _not_modified_in_module_call;
        }

      private:
        std::optional<bool> _not_modified_in_module_call;
      };

      struct memory_getput : memory_transfer_type
      {
      public:
        memory_getput ( util::position_type const&
                      , std::string const& global
                      , std::string const& local
                      , ::gspc::we::type::property::type const&
                      , std::optional<bool> const&
                      , std::optional<bool> const&
                      );
        std::optional<bool> const& not_modified_in_module_call() const
        {
          return _not_modified_in_module_call;
        }

      private:
        std::optional<bool> _not_modified_in_module_call;
      };

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream&, memory_get const&);
        void dump (::gspc::util::xml::xmlstream&, memory_put const&);
        void dump (::gspc::util::xml::xmlstream&, memory_getput const&);
      }
    }
