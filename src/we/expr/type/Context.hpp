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

#pragma once

#include <we/expr/type/Path.hpp>
#include <we/expr/type/Type.hpp>

#include <boost/optional.hpp>

namespace expr
{
  namespace type
  {
    //! Struct-aware type context.
    struct Context
    {
    public:
      //! Remember that the type at the position \a path is \a type.
      //! Example:
      //!   bind ({"v"}, Int{}) -> *at ({"v"}) == Int{}
      //!
      //! Existing 'Struct's are extended.
      //! Example:
      //!   bind ({"s", "a"}, Ta) -> *at ({"s"}) == Struct [a::Ta]
      //!   bind ({"s", "b"}, Tb) -> *at ({"s"}) == Struct [a::Ta, b::Tb]
      //!
      //! Existing non-'Struct' can not be extented.
      //! Example:
      //!   bind ({"x"}, Int{}) -> *at ({"x"}) == Int{}
      //!   bind ({"x"}, T)     -> throws if T != Int, *at ({"x"}) == Int{}
      //!
      //! Existing nodes with 'existing_type' are overwritten by
      //! 'assign_result (existing_type, type)'
      //! Example:
      //!   bind ({"n"}, Ta)  -> *at ({"n"}) == Struct [a::Ta]
      //!   bind ({"n"}, Any) -> *at ({"n"}) == Struct [a::Any]
      //!   bind ({"n"}, Tb)  -> *at ({"n"}) == Struct [a::Tb]
      //!   bind ({"n"}, Tc)  -> throws unless values of type Tc can be
      //!                        assigned to values of type y::Tb
      Type bind (Path const& path, Type const& type);

      //! Retrieve the remembered type at the position \a path or
      //! return None if no type has been remembered at the position
      //! \a path.
      //! Example:
      //!   Context c;
      //!   c.bind (path, type);
      //!   assert (!!c.at (path));
      //!   assert (*c.at (path) == type);
      //!   assert (!c.at (different_path));
      ::boost::optional<Type> at (Path const& path) const noexcept;

    private:
      //! \note "Misuse" of 'Struct' instead of 'map<path.front(), Type>'
      //!
      //! As a consequence `at ({})` can be used to retrieve the full
      //! universe of bound types and `bind ({}, ...)` can be used to
      //! overwrite the full universe of types.
      Type _root = Struct {{}};
    };
  }
}
