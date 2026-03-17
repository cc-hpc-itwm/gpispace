// Copyright (C) 2013,2020-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/value/path/append.hpp>




      namespace gspc::pnet::type::value::path
      {
        append::append ( std::list<std::string>& path
                       , std::string const& key
                       )
          : _path (path)
        {
          _path.push_back (key);
        }
        append::~append()
        {
          _path.pop_back();
        }
        append::operator std::list<std::string>&() const
        {
          return _path;
        }
      }
