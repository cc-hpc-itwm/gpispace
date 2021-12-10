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

#include <we/expr/eval/context.hpp>
#include <we/expr/type/Context.hpp>
#include <we/type/Expression.hpp>

#include <boost/serialization/string.hpp>

#include <string>

namespace we
{
  namespace type
  {
    class MemoryBufferInfo
    {
    public:
      //! For deserialization only.
      MemoryBufferInfo();

      MemoryBufferInfo
        (std::string const& size, std::string const& alignment);

      unsigned long size (expr::eval::context const&) const;
      unsigned long alignment (expr::eval::context const&) const;

      void assert_correct_expression_types
        (expr::type::Context const&) const;

    private:
      Expression _size;
      Expression _alignment;

      friend class ::boost::serialization::access;

      template<typename Archive>
      void serialize (Archive & ar, unsigned int)
      {
        ar & _size;
        ar & _alignment;
      }
    };
  }
}
