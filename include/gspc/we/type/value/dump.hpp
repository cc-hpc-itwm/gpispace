// Copyright (C) 2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/value.hpp>

#include <gspc/util/xml.fwd.hpp>



    namespace gspc::pnet::type::value
    {
      util::xml::xmlstream& dump ( util::xml::xmlstream&
                                      , value_type const&
                                      );
    }
