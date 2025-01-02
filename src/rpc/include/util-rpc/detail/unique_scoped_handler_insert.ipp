// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
            ( unique_scoped_handler_insert<Handlers>&& other
            ) noexcept
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
