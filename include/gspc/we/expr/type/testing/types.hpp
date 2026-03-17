// Copyright (C) 2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/expr/type/Type.hpp>

#include <gspc/we/type/signature.hpp>

#include <string>
#include <vector>



    namespace gspc::we::expr::type::testing
    {
      struct TypeDescription
      {
        Type type;
        std::string expression;
        pnet::type::signature::signature_type signature;
      };
      std::ostream& operator<< (std::ostream&, TypeDescription const&);

      std::vector<TypeDescription> all_types();
      std::vector<TypeDescription> all_types_except (Type);
    }
