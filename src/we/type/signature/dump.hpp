// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/signature.hpp>

#include <fhg/util/xml.fwd.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
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

      void dump_to (fhg::util::xml::xmlstream&, structured_type const&);
    }
  }
}
