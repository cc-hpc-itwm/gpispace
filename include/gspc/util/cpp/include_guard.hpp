// Copyright (C) 2010,2013,2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/ostream/modifier.hpp>

#include <iosfwd>
#include <string>




      namespace gspc::util::cpp::include_guard
      {
        class open : public ostream::modifier
        {
        public:
          open (std::string const&);
          std::ostream& operator() (std::ostream&) const override;
        private:
          const std::string _name;
        };

        class close : public ostream::modifier
        {
        public:
          std::ostream& operator() (std::ostream&) const override;
        };
      }
