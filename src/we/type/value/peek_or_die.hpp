// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>
#include <we/type/value.hpp>
#include <we/type/value/from_value.hpp>
#include <we/type/value/peek.hpp>

#include <boost/optional.hpp>

#include <list>
#include <stdexcept>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace error
      {
        auto GSPC_DLLEXPORT could_not_peek
          ( value_type const& store
          , std::list<std::string> const& path
          ) -> std::logic_error
          ;
      }

      template<typename T>
        T peek_or_die ( value_type const& store
                      , std::list<std::string> const& path
                      )
      {
        ::boost::optional<value_type const&> const value (peek (path, store));

        if (!value)
        {
          throw error::could_not_peek (store, path);
        }

        return from_value<T> (value.get());
      }
    }
  }
}
