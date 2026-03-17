// Copyright (C) 2013-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/signature.hpp>

#include <gspc/detail/export.hpp>

#include <gspc/util/ostream/modifier.hpp>

#include <iosfwd>



    namespace gspc::pnet::type::signature
    {
      class GSPC_EXPORT show : public util::ostream::modifier
      {
      public:
        show (signature_type const&);
        std::ostream& operator() (std::ostream&) const override;
      private:
        signature_type const& _signature;
      };
    }
