// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/value.hpp>

#include <fhg/util/xml.fwd.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      fhg::util::xml::xmlstream& dump ( fhg::util::xml::xmlstream&
                                      , value_type const&
                                      );
    }
  }
}
