// Copyright (C) 2019,2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once


#include <list>
#include <string>

namespace
{
  //! \note restrict target and module name length to ensure
  //        lib/cpp filenames generated are not too long
  constexpr auto const MAX_TARGET_LEN = 16;
  constexpr auto const MAX_TARGETS = 64;
  constexpr auto const MAX_MODNAME_LEN = 32;

  std::list<std::string> gen_valid_targets (size_t max);

  std::string gen_pnet_with_multi_modules
    ( std::list<std::string> const& preference_targets
    , std::list<std::string> const& module_targets
    , std::string const& module_name
    , std::optional<std::string> const& add_trans_content
    );

  struct pnet_with_multi_modules
  {
    std::string const module_name;
    std::string const pnet_xml;

    pnet_with_multi_modules
      ( std::list<std::string> const& pref_targets
      , std::list<std::string> const& mod_targets
      , std::string const& mod_name
      , std::optional<std::string> add_to_pnet
        = std::nullopt
      );

    pnet_with_multi_modules
      ( std::list<std::string> const& pref_targets
      , std::list<std::string> const& mod_targets
      );
  };
}

#include <test/xml/parse/preference_list_with_modules_helpers.ipp>
#include <optional>
