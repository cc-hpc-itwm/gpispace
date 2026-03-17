// Copyright (C) 2019-2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/random.hpp>

#include <gspc/util/join.hpp>

#include <fmt/core.h>
#include <optional>

namespace
{
  std::list<std::string> gen_valid_targets (size_t const max)
  {
    struct generator
    {
      std::string operator()() const
      {
        return gspc::testing::random_identifier (MAX_TARGET_LEN);
      }
    };

    return gspc::testing::randoms<std::list<std::string>>
      ( gspc::testing::random<std::size_t>{} (max, 1)
      , gspc::testing::unique_random<std::string, generator>{}
      );
  }

  std::string gen_pnet_with_multi_modules
    ( std::list<std::string> const& preference_targets
    , std::list<std::string> const& module_targets
    , std::string const& module_name
    , std::optional<std::string> const& add_trans_content
    )
  {
    std::string pnet_target_list = "";
    if (!preference_targets.empty())
    {
      pnet_target_list =
        ( "<preferences>\n<target>"
          + gspc::util::join ( preference_targets
                            , "</target><target>"
                            ).string()
          + "</target>\n</preferences>"
        );
    }

    if (module_targets.empty())
    {
      throw std::logic_error ("no module targets provided for pnet");
    }

    std::string const pnet_module_prefix
      ( "<module name=\""
        + module_name + "\""
        + " function=\"func()\""
        + " target=\""
      );
    std::string const pnet_module_suffix
      ("\"><code><![CDATA[ return; ]]></code></module>\n");

    std::string const pnet_module_list
      ( pnet_module_prefix
        + gspc::util::join ( module_targets
                          , pnet_module_suffix
                            + pnet_module_prefix
                          ).string()
        + pnet_module_suffix
      );

    return fmt::format ( R"EOS(
        <defun name="n_preferences">
          <modules>
          {0}
          {2}
          {1}
          </modules>
        </defun>)EOS"
                  , pnet_target_list
                  , pnet_module_list
                  , add_trans_content.value_or ("")
        );
  }

  pnet_with_multi_modules::pnet_with_multi_modules
    ( std::list<std::string> const& pref_targets
    , std::list<std::string> const& mod_targets
    , std::string const& mod_name
    , std::optional<std::string> const add_to_pnet
    )
    : module_name (mod_name)
    , pnet_xml ( gen_pnet_with_multi_modules
                  ( pref_targets
                  , mod_targets
                  , mod_name
                  , add_to_pnet
                  )
                )
  {}

  pnet_with_multi_modules::pnet_with_multi_modules
    ( std::list<std::string> const& pref_targets
    , std::list<std::string> const& mod_targets
    )
    : pnet_with_multi_modules
        ( pref_targets
        , mod_targets
        , gspc::testing::random_identifier
          (MAX_MODNAME_LEN)
        , std::nullopt
        )
  {}
}
