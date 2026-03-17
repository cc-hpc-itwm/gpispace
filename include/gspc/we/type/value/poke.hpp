// Copyright (C) 2013,2015,2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/type/value.hpp>



    namespace gspc::pnet::type::value
    {
      GSPC_EXPORT value_type& poke
        ( std::list<std::string>::const_iterator const&
        , std::list<std::string>::const_iterator const&
        , value_type&
        , value_type const&
        );
      GSPC_EXPORT value_type& poke
        ( std::list<std::string> const& path
        , value_type& node
        , value_type const& value
        );
      GSPC_EXPORT value_type& poke
        (std::string const&, value_type&, value_type const&);
    }
