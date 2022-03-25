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

#include <we/expr/type/Type.hpp>

#include <we/type/signature.hpp>

#include <string>
#include <vector>

namespace expr
{
  namespace type
  {
    namespace testing
    {
      struct TypeDescription
      {
        Type type;
        std::string expression;
        ::pnet::type::signature::signature_type signature;
      };
      std::ostream& operator<< (std::ostream&, TypeDescription const&);

      std::vector<TypeDescription> all_types();
      std::vector<TypeDescription> all_types_except (Type);
    }
  }
}
