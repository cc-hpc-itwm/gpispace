// Copyright (C) 2018,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/ostream/redirect.hpp>

#include <ostream>
#include <string>


  namespace gspc::logging
  {
    struct ostream_redirect : public gspc::util::ostream::redirect
    {
      template<typename Emitter>
        ostream_redirect (std::ostream&, Emitter&, std::string category);
    };
  }


#include <gspc/logging/ostream_redirect.ipp>
