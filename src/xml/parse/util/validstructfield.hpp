// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem/path.hpp>

namespace xml
{
  namespace parse
  {
    std::string validate_field_name ( std::string const&
                                    , ::boost::filesystem::path const&
                                    );
  }
}
