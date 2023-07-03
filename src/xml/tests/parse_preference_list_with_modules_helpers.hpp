// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/optional.hpp>

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
    , ::boost::optional<std::string> const& add_trans_content
    );

  struct pnet_with_multi_modules
  {
    std::string const module_name;
    std::string const pnet_xml;

    pnet_with_multi_modules
      ( std::list<std::string> const& pref_targets
      , std::list<std::string> const& mod_targets
      , std::string const& mod_name
      , ::boost::optional<std::string> add_to_pnet
        = ::boost::none
      );

    pnet_with_multi_modules
      ( std::list<std::string> const& pref_targets
      , std::list<std::string> const& mod_targets
      );
  };
}

#include <xml/tests/parse_preference_list_with_modules_helpers.ipp>
