// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/signature.hpp>

#include <gspc/detail/dllexport.hpp>

#include <util-generic/ostream/modifier.hpp>

#include <iosfwd>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      class GSPC_DLLEXPORT show : public fhg::util::ostream::modifier
      {
      public:
        show (signature_type const&);
        std::ostream& operator() (std::ostream&) const override;
      private:
        signature_type const& _signature;
      };
    }
  }
}
