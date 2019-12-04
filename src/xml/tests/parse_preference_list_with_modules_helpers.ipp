#include <util-generic/testing/random.hpp>

namespace
{
  std::list<std::string> gen_valid_targets (size_t const max)
  {
    size_t _max = ( fhg::util::testing::unique_random<size_t>{}()
                   % max
                 ) + 1;

    std::list<std::string> _targets;
    std::function<std::string()> generate
      ([] { return fhg::util::testing::random_identifier(); });
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
    std::string const pnet_target_list
      ( "<target>"
        + fhg::util::join_reference < std::list<std::string>
                                    , std::string
                                    >
          (preference_targets, "</target><target>").string()
        + "</target>"
      );

    std::string const pnet_module_prefix
      ( "<module name=\""
        + module_name + "\""
        + " function=\"func()\""
        + " target=\""
      );
    std::string const pnet_module_suffix ("\"></module>\n");
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
          <preferences>
          %1%
          </preferences>
          %3%
          %2%
        </defun>)EOS")
        % pnet_target_list
        % pnet_module_list
        % (add_trans_content ? *add_trans_content : "")
      ).str();
  }

  constexpr auto const MAX_TARGETS = 1000;

  struct pnet_with_multi_modules
  {
    std::string const pnet_xml;

    pnet_with_multi_modules
      ( std::list<std::string> const& pref_targets
      , std::list<std::string> const& mod_targets
      , std::string const& mod_name
      , boost::optional<std::string> const add_to_pnet 
        = boost::none
      )
      : pnet_xml ( gen_pnet_with_multi_modules
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
          , fhg::util::testing::random_identifier()
          , boost::none
          )
    {}
  };
}
