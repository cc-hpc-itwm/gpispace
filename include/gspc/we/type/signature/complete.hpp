// Copyright (C) 2013,2015,2020-2021,2023,2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iosfwd>
#include <string>



    namespace gspc::pnet::type::signature
    {
      class complete
      {
      public:
        complete (std::string const&);
        std::string const& tname() const;
      private:
        std::string const& _tname;
      };
      std::ostream& operator<< (std::ostream&, complete const&);
    }
