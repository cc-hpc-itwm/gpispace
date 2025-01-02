// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
