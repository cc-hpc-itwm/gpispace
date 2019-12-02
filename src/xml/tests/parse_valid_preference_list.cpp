#include <boost/test/unit_test.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/state.hpp>

#include <we/type/activity.hpp>
#include <we/type/net.hpp>

#include <util-generic/testing/random.hpp>
#include <util-generic/testing/random/string.hpp>
#include <util-generic/testing/require_exception.hpp>
#include <util-generic/testing/printer/list.hpp>

#include <util-generic/join.hpp>

#include <string>
#include <unordered_set>
#include <list>
#include <functional>

#include <boost/range/adaptors.hpp>

namespace
{
  constexpr auto const MAX_TARGET_PREFERENCES = 10000;

  std::list<std::string> gen_valid_targets()
  {
    size_t max = ( fhg::util::testing::unique_random<size_t>{}()
                   % MAX_TARGET_PREFERENCES
                 ) + 1;

    std::list<std::string> _targets;
    std::function<std::string()> generate
      ([] { return fhg::util::testing::random_identifier(); });
    std::generate_n
      ( std::back_inserter (_targets)
        , max
        , generate
      );

    _targets.sort();
    _targets.unique();

    return _targets;
  }

}
