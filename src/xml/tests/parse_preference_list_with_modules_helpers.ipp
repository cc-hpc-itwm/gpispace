#include <util-generic/testing/random.hpp>

namespace
{
  //! \note restricting target and module name length to ensure
  //        lib/cpp filenames generated are not too long
  constexpr auto const MAX_TARGET_LEN = 16;
  constexpr auto const MAX_MODNAME_LEN = 32;
  constexpr auto const MAX_TARGETS = 64;

  std::list<std::string> gen_valid_targets (size_t const max)
  {
    size_t _max = ( fhg::util::testing::unique_random<size_t>{}()
                   % max
                 ) + 1;

    std::list<std::string> _targets;
    std::function<std::string()> generate
      ( []
        {
          return fhg::util::testing::random_identifier (MAX_TARGET_LEN);
        }
      );
    std::generate_n
      ( std::back_inserter (_targets)
        , _max
        , generate
      );

    _targets.sort();
    _targets.unique();

    return _targets;
  }

  std::string gen_pnet_with_multi_modules
    ( std::list<std::string> const& preference_targets
    , std::list<std::string> const& module_targets
    , std::string const& module_name
    , boost::optional<std::string> const& add_trans_content
    )
  {
    std::string pnet_target_list = "";
    if (!preference_targets.empty())
    {
      pnet_target_list =
        ( "<preferences>\n<target>"
          + fhg::util::join_reference < std::list<std::string>
                                      , std::string
                                      >
            (preference_targets, "</target><target>").string()
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
        + fhg::util::join_reference < std::list<std::string>
                                    , std::string
                                    >
          ( module_targets
          , pnet_module_suffix + pnet_module_prefix
          )
          .string()
        + pnet_module_suffix
      );

    return
      ( boost::format (R"EOS(
        <defun name="n_preferences">
          %1%
          %3%
          %2%
        </defun>)EOS")
        % pnet_target_list
        % pnet_module_list
        % (add_trans_content ? *add_trans_content : "")
      ).str();
  }

  struct pnet_with_multi_modules
  {
    std::string const module_name;
    std::string const pnet_xml;

    pnet_with_multi_modules
      ( std::list<std::string> const& pref_targets
      , std::list<std::string> const& mod_targets
      , std::string const& mod_name
      , boost::optional<std::string> const add_to_pnet
        = boost::none
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

    pnet_with_multi_modules
      ( std::list<std::string> const& pref_targets
      , std::list<std::string> const& mod_targets
      )
      : pnet_with_multi_modules
          ( pref_targets
          , mod_targets
          , fhg::util::testing::random_identifier
            (MAX_MODNAME_LEN)
          , boost::none
          )
    {}
  };
}
