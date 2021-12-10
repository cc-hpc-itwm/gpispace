// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <util-generic/testing/random.hpp>

#include <util-generic/join.hpp>

#include <boost/format.hpp>

namespace
{
  std::list<std::string> gen_valid_targets (size_t const max)
  {
    struct generator
    {
      std::string operator()() const
      {
        return fhg::util::testing::random_identifier (MAX_TARGET_LEN);
      }
    };

    return fhg::util::testing::randoms<std::list<std::string>>
      ( fhg::util::testing::random<std::size_t>{} (max, 1)
      , fhg::util::testing::unique_random<std::string, generator>{}
      );
  }

  std::string gen_pnet_with_multi_modules
    ( std::list<std::string> const& preference_targets
    , std::list<std::string> const& module_targets
    , std::string const& module_name
    , ::boost::optional<std::string> const& add_trans_content
    )
  {
    std::string pnet_target_list = "";
    if (!preference_targets.empty())
    {
      pnet_target_list =
        ( "<preferences>\n<target>"
          + fhg::util::join ( preference_targets
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
        + fhg::util::join ( module_targets
                          , pnet_module_suffix
                            + pnet_module_prefix
                          ).string()
        + pnet_module_suffix
      );

    return
      ( ::boost::format (R"EOS(
        <defun name="n_preferences">
          <modules>
          %1%
          %3%
          %2%
          </modules>
        </defun>)EOS")
        % pnet_target_list
        % pnet_module_list
        % add_trans_content.value_or ("")
      ).str();
  }

  pnet_with_multi_modules::pnet_with_multi_modules
    ( std::list<std::string> const& pref_targets
    , std::list<std::string> const& mod_targets
    , std::string const& mod_name
    , ::boost::optional<std::string> const add_to_pnet
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
        , fhg::util::testing::random_identifier
          (MAX_MODNAME_LEN)
        , ::boost::none
        )
  {}
}
