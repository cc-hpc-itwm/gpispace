// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/expr/type/Path.hpp>
#include <we/expr/type/Type.hpp>

namespace expr
{
  namespace type
  {
    //! Returns \a rhs if a value of type \a rhs can be assigned to a
    //! value of type \a lhs and throws if such an assignment is
    //! impossible.
    //! Example:
    //!   assign_result (p, Int{}, Int{}) == Int{}
    //!
    //! The parameter \a path is used in error messages to better hint
    //! users.
    //!
    //! Heterogenous containers are handled.
    //! Example:
    //!   assign_result (p, List (T), List (Any)) == List (Any)
    //!   assign_result (p, List (Any), List (T)) == List (T)
    //!   assign_result (p, List (Ta), List ({Ta, Tb})) == List ({Ta, Tb})
    //!   assign_result (p, List ({Ta, Tb}), List (Ta)) == List (Ta)
    //!   assign_result (p, List ({Ta, Tb}), List ({Ta, Tc})) == List ({Ta, Tc})
    //!
    //! Singleton containers are considered to be homogeneous.
    //!   assign_result (p, List (String), List (Int)) -> throws "String != Int"
    //!
    //! 'Struct's are handled.
    //! Example:
    //!   assign_result (p, Struct[a::Int], Struct[b::Int]) -> throws "a != b"
    //!   assign_result (p, Struct[a::Int], Struct[a::String]) -> throws "String != Int"
    //!   assign_result (p, Struct[a::Int], Struct[a::Any]) -> Struct[a::Any]
    //!
    //! Throws when 'Struct' fields are missing or given additionally.
    //! Example:
    //!   assign_result (p, Struct[], Struct[a::T]) -> throws "additional a"
    //!   assign_result (p, Struct[a::T], Struct[]) -> throws "missing a"
    Type assign_result (Path path, Type const& lhs, Type const& rhs);
  }
}
