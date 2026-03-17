// Copyright (C) 2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iosfwd>
#include <list>
#include <set>
#include <string>



    namespace gspc::pnet::type::value
    {
#define NAME(_name) std::string const& _name()

      NAME (CONTROL);
      NAME (BOOL);
      NAME (INT);
      NAME (LONG);
      NAME (UINT);
      NAME (ULONG);
      NAME (FLOAT);
      NAME (DOUBLE);
      NAME (CHAR);
      NAME (STRING);
      NAME (BITSET);
      NAME (BYTEARRAY);
      NAME (BIGINT);
      NAME (SHARED);
      NAME (LIST);
      NAME (SET);
      NAME (MAP);
      NAME (STRUCT);

#undef NAME

      std::list<std::string> type_names();
    }
