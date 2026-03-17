// Copyright (C) 2010,2013,2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>


  namespace gspc::xml::parse
  {
    std::string validate_field_name ( std::string const&
                                    , std::filesystem::path const&
                                    );
  }
