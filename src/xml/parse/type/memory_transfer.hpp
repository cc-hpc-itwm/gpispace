// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <xml/parse/type/with_position_of_definition.hpp>

#include <xml/parse/util/position.fwd.hpp>

#include <we/type/property.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <boost/optional.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct memory_transfer_type : with_position_of_definition
      {
      public:
        memory_transfer_type ( util::position_type const&
                             , std::string const& global
                             , std::string const& local
                             , we::type::property::type const&
                             , ::boost::optional<bool> const&
                             );

        std::string const& global() const
        {
          return _global;
        }
        std::string const& local() const
        {
          return _local;
        }
        we::type::property::type const& properties() const
        {
          return _properties;
        }
        ::boost::optional<bool> const& allow_empty_ranges() const;

      private:
        std::string _global;
        std::string _local;
        we::type::property::type _properties;
        ::boost::optional<bool> _allow_empty_ranges;
      };

      struct memory_get : memory_transfer_type
      {
      public:
        memory_get ( util::position_type const&
                   , std::string const& global
                   , std::string const& local
                   , we::type::property::type const&
                   , ::boost::optional<bool> const&
                   );
      };

      struct memory_put : memory_transfer_type
      {
      public:
        memory_put ( util::position_type const&
                   , std::string const& global
                   , std::string const& local
                   , we::type::property::type const&
                   , ::boost::optional<bool> const&
                   , ::boost::optional<bool> const&
                   );
        ::boost::optional<bool> const& not_modified_in_module_call() const
        {
          return _not_modified_in_module_call;
        }

      private:
        ::boost::optional<bool> _not_modified_in_module_call;
      };

      struct memory_getput : memory_transfer_type
      {
      public:
        memory_getput ( util::position_type const&
                      , std::string const& global
                      , std::string const& local
                      , we::type::property::type const&
                      , ::boost::optional<bool> const&
                      , ::boost::optional<bool> const&
                      );
        ::boost::optional<bool> const& not_modified_in_module_call() const
        {
          return _not_modified_in_module_call;
        }

      private:
        ::boost::optional<bool> _not_modified_in_module_call;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, memory_get const&);
        void dump (::fhg::util::xml::xmlstream&, memory_put const&);
        void dump (::fhg::util::xml::xmlstream&, memory_getput const&);
      }
    }
  }
}
