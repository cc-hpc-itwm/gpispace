// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/Expression.hpp>
#include <we/type/memory_transfer.hpp>

#include <exception>
#include <fmt/core.h>
#include <stdexcept>

namespace we
{
  namespace type
  {
    memory_transfer::memory_transfer() = default;

    memory_transfer::memory_transfer
      ( std::string const& global
      , std::string const& local
      , ::boost::optional<bool> const& not_modified_in_module_call
      , bool allow_empty_ranges
      )
        : _global (global)
        , _local (local)
        , _not_modified_in_module_call (not_modified_in_module_call)
        , _allow_empty_ranges (allow_empty_ranges)
    {}
    std::string const& memory_transfer::global() const
    {
      return _global;
    }
    std::string const& memory_transfer::local() const
    {
      return _local;
    }
    ::boost::optional<bool> const& memory_transfer::not_modified_in_module_call() const
    {
      return _not_modified_in_module_call;
    }
    bool const& memory_transfer::allow_empty_ranges() const
    {
      return _allow_empty_ranges;
    }
    void memory_transfer::assert_correct_expression_types
      (expr::type::Context const& context) const
    {
      try
      {
        expr::type::Struct const local_range_type
          ( { {"buffer", expr::type::String{}}
            , {"offset", expr::type::ULong{}}
            , {"size", expr::type::ULong{}}
            }
          )
          ;
        expr::type::List const expected (local_range_type);

        Expression (_local).assert_type (expected, context);
      }
      catch (...)
      {
        std::throw_with_nested
          ( std::runtime_error
            { fmt::format ("In the <local> expression '{}'", _local)
            }
          );
      }

      try
      {
        expr::type::Struct const global_handle_type
          ( { {"name", expr::type::String{}}
            }
          )
          ;
        expr::type::Struct const global_range_type
          ( { {"handle", global_handle_type}
            , {"offset", expr::type::ULong{}}
            , {"size", expr::type::ULong{}}
            }
          )
          ;
        expr::type::List const expected (global_range_type);

        Expression (_global).assert_type (expected, context);
      }
      catch (...)
      {
        std::throw_with_nested
          ( std::runtime_error
            { fmt::format ("In the <global> expression '{}'", _global)
            }
          );
      }
    }
  }
}
