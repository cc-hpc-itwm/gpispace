// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/expr/type/testing/types.hpp>

#include <we/type/signature/show.hpp>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>

namespace expr
{
  namespace type
  {
    namespace testing
    {
      std::ostream& operator<< (std::ostream& os, TypeDescription const& td)
      {
        return os
          << td.type
          << ", " << td.expression
          << ", " << pnet::type::signature::show (td.signature)
          ;
      }

      std::vector<TypeDescription> all_types()
      {
        return
          { {Control{}, "[]", "control"}
          , {Boolean{}, "true", "bool"}
          , {Int{}, "0", "int"}
          , {Long{}, "0l", "long"}
          , {UInt{}, "0u", "unsigned int"}
          , {ULong{}, "0ul", "unsigned long"}
          , {Float{}, "0f", "float"}
          , {Double{}, "0.0", "double"}
          , {Char{}, "'c'", "char"}
          , {String{}, "\"\"", "string"}
          , {Bitset{}, "{}", "bitset"}
          , {Bytearray{}, "y()", "bytearray"}
          , {List {Any()}, "List()", "list"}
          , {Set {Any()}, "Set{}", "set"}
          , {Map {Any(), Any()}, "Map[]", "map"}
          , {Struct {{}}, "Struct[]"
            , std::make_pair ( std::string ("struct")
                             , std::list<pnet::type::signature::field_type>{}
                             )
            }
          };
      }

      std::vector<TypeDescription> all_types_except (Type type)
      {
        auto all (all_types());
        decltype (all) filtered;

        std::copy_if ( std::begin (all), std::end (all)
                     , std::inserter (filtered, std::end (filtered))
                     , [&type] (auto const& te)
                       {
                         return te.type != type;
                       }
                     );

        return filtered;
      }
    }
  }
}
