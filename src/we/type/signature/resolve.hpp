// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/signature.hpp>

#include <boost/optional.hpp>

#include <functional>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      using resolver_type = std::function< ::boost::optional<signature_type> (const std::string &)>;

      signature_type resolve (structured_type const&, resolver_type const&);
    }
  }
}
