// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/dllexport.hpp>

#include <list>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace path
      {
        class GSPC_DLLEXPORT append
        {
        public:
          append (std::list<std::string>&, std::string const&);
          ~append();
          append (append const&) = delete;
          append (append&&) = delete;
          append& operator= (append const&) = delete;
          append& operator= (append&&) = delete;
          operator std::list<std::string>&() const;
        private:
          std::list<std::string>& _path;
        };
      }
    }
  }
}
