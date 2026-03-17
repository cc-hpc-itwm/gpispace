// Copyright (C) 2013,2015,2020-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>



    namespace gspc::pnet::type::value
    {
      template<typename T> inline std::string const& name_of (T const&);
    }



#include <gspc/we/type/value/name_of.ipp>
