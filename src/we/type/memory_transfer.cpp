// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <we/type/Expression.hpp>
#include <we/type/memory_transfer.hpp>

#include <util-generic/nest_exceptions.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace we
{
  namespace type
  {
    memory_transfer::memory_transfer() = default;

    memory_transfer::memory_transfer
      ( std::string const& global
      , std::string const& local
      , boost::optional<bool> const& not_modified_in_module_call
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
    boost::optional<bool> const& memory_transfer::not_modified_in_module_call() const
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
      fhg::util::nest_exceptions<std::runtime_error>
        ( [&]
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
        , str ( boost::format ("In the <local> expression '%1%'")
              % _local
              )
        );

      fhg::util::nest_exceptions<std::runtime_error>
        ( [&]
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
        , str ( boost::format ("In the <global> expression '%1%'")
              % _global
              )
        );
    }
  }
}
