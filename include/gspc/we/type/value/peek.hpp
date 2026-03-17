// Copyright (C) 2013,2015,2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/type/value.hpp>

#include <functional>
#include <optional>



    namespace gspc::pnet::type::value
    {
      GSPC_EXPORT std::optional<std::reference_wrapper<value_type const>>
      peek ( std::list<std::string>::const_iterator const&
           , std::list<std::string>::const_iterator const&
           , value_type const&
           );
      GSPC_EXPORT std::optional<std::reference_wrapper<value_type const>>
      peek (std::list<std::string> const& path, value_type const& node);
      GSPC_EXPORT std::optional<std::reference_wrapper<value_type const>>
      peek (std::string const&, value_type const&);

      GSPC_EXPORT std::optional<std::reference_wrapper<value_type>>
      peek ( std::list<std::string>::const_iterator const&
           , std::list<std::string>::const_iterator const&
           , value_type&
           );
      GSPC_EXPORT std::optional<std::reference_wrapper<value_type>>
      peek (std::list<std::string> const& path, value_type& node);
      GSPC_EXPORT std::optional<std::reference_wrapper<value_type>>
      peek (std::string const&, value_type&);
    }
