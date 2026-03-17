// Copyright (C) 2013,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/signature/is_literal.hpp>
#include <gspc/we/type/shared.hpp>
#include <gspc/we/type/value/name.hpp>

#include <iostream>
#include <set>



    namespace gspc::pnet::type::signature
    {
      namespace
      {
        std::set<std::string> init_literal_names()
        {
          std::set<std::string> ln;

          ln.insert (value::CONTROL());
          ln.insert (value::BOOL());
          ln.insert (value::INT());
          ln.insert (value::LONG());
          ln.insert (value::UINT());
          ln.insert (value::ULONG());
          ln.insert (value::FLOAT());
          ln.insert (value::DOUBLE());
          ln.insert (value::CHAR());
          ln.insert (value::STRING());
          ln.insert (value::BITSET());
          ln.insert (value::BYTEARRAY());
          ln.insert (value::BIGINT());
          ln.insert (value::SHARED());
          ln.insert (value::LIST());
          ln.insert (value::SET());
          ln.insert (value::MAP());

          ln.insert ("stack");

          return ln;
        }

        bool is_shared_type (std::string const& tname)
        {
          // shared_PLACENAME is a literal type
          return we::type::shared::cleanup_place (tname).has_value();
        }
      }

      bool is_literal (std::string const& tname)
      {
        static std::set<std::string> ln (init_literal_names());

        return ln.find (tname) != ln.end() || is_shared_type (tname);
      }
    }
