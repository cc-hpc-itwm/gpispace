// Copyright (C) 2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/signature.hpp>

#include <gspc/util/xml.fwd.hpp>



    namespace gspc::pnet::type::signature
    {
      class dump
      {
      public:
        dump (structured_type const&);
        std::ostream& operator() (std::ostream&) const;
      private:
        structured_type const& _structured;
      };
      std::ostream& operator<< (std::ostream&, dump const&);

      void dump_to (util::xml::xmlstream&, structured_type const&);
    }
