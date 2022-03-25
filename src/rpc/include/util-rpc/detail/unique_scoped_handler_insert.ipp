// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <util-rpc/common.hpp>

#include <utility>

namespace fhg
{
  namespace rpc
  {
    namespace detail
    {
      template<typename Handlers>
        unique_scoped_handler_insert<Handlers>::unique_scoped_handler_insert
            ( Handlers& handlers
            , typename Handlers::key_type name
            , typename Handlers::mapped_type function
            )
          : _handlers (handlers)
          , _name (::boost::none)
      {
        if (!_handlers.emplace (name, std::move (function)).second)
        {
          throw rpc::error::duplicate_function (std::move (name));
        }

        _name = std::move (name);
      }
      template<typename Handlers>
        unique_scoped_handler_insert<Handlers>::unique_scoped_handler_insert
            (unique_scoped_handler_insert<Handlers>&& other)
          : _handlers (std::move (other._handlers))
          , _name (::boost::none)
      {
        std::swap (_name, other._name);
      }
      template<typename Handlers>
        unique_scoped_handler_insert<Handlers>::~unique_scoped_handler_insert()
      {
        if (_name)
        {
          _handlers.erase (*_name);
        }
      }
    }
  }
}
