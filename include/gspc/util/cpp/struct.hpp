// Copyright (C) 2013,2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/indenter.hpp>
#include <gspc/util/ostream/modifier.hpp>

#include <iosfwd>
#include <optional>
#include <string>




      namespace gspc::util::cpp::structure
      {
        class open : public ostream::modifier
        {
        public:
          open (gspc::util::indenter&);
          open (gspc::util::indenter&, std::string const&);
          std::ostream& operator() (std::ostream&) const override;

        private:
          gspc::util::indenter& _indent;
          const std::optional<std::string> _tag;
        };

        class close : public ostream::modifier
        {
        public:
          close (gspc::util::indenter&);
          std::ostream& operator() (std::ostream&) const override;

        private:
          gspc::util::indenter& _indent;
        };
      }
