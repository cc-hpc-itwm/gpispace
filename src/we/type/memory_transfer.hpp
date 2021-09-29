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

#pragma once

#include <we/expr/type/Context.hpp>

#include <boost/optional/optional.hpp>
#include <boost/serialization/access.hpp>

#include <string>

namespace we
{
  namespace type
  {
    struct memory_transfer
    {
    public:
      //! \note serialization only
      memory_transfer();

      memory_transfer
        ( std::string const& global
        , std::string const& local
        , boost::optional<bool> const& not_modified_in_module_call
        , bool allow_empty_ranges
        );
      std::string const& global() const;
      std::string const& local() const;
      boost::optional<bool> const& not_modified_in_module_call() const;
      bool const& allow_empty_ranges() const;

      void assert_correct_expression_types
        (expr::type::Context const&) const;

    private:
      std::string _global;
      std::string _local;
      boost::optional<bool> _not_modified_in_module_call;
      bool _allow_empty_ranges;

      friend class boost::serialization::access;
      template<class Archive> void serialize (Archive&, unsigned int);
    };
  }
}

#include <we/type/memory_transfer.ipp>
