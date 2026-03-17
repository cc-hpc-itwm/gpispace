// Copyright (C) 2015,2021,2023-2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>
#include <gspc/we/type/value.hpp>
#include <gspc/we/type/value/from_value.hpp>
#include <gspc/we/type/value/peek.hpp>

#include <optional>

#include <list>
#include <stdexcept>



    namespace gspc::pnet::type::value
    {
      namespace error
      {
        auto GSPC_EXPORT could_not_peek
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
        std::optional<std::reference_wrapper<value_type const>> const value (peek (path, store));

        if (!value)
        {
          throw error::could_not_peek (store, path);
        }

        return ::boost::get<T> (value->get());
      }
    }
