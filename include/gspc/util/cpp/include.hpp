// Copyright (C) 2010,2013,2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/ostream/modifier.hpp>

#include <filesystem>

#include <iosfwd>
#include <string>



    namespace gspc::util::cpp
    {
      class include : public ostream::modifier
      {
      public:
        include (std::string const&);
        std::ostream& operator() (std::ostream&) const override;
      private:
        std::string const& _fname;
      };
    }
