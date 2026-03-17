// Copyright (C) 2013,2015,2021-2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <list>
#include <string>




      namespace gspc::pnet::type::value::path
      {
        class GSPC_EXPORT append
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
