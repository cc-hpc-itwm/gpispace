// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnetv_use_sequence
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <test/source_directory.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/read_lines.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

#include <set>

BOOST_AUTO_TEST_CASE (pnetv_use_sequence)
{
  boost::program_options::options_description options_description;

  options_description.add (test::options::source_directory());
  options_description.add (gspc::options::installation());

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser
      ( boost::unit_test::framework::master_test_suite().argc
      , boost::unit_test::framework::master_test_suite().argv
      )
    . options (options_description).run()
    , vm
    );

  vm.notify();

  fhg::util::temporary_path const _temporary_path;
  boost::filesystem::path const temporary_path (_temporary_path);

  fhg::util::system_with_blocked_SIGCHLD
    (( boost::format ("%1%/bin/pnetc -i %2% | %1%/bin/pnetv -i - > %3%")
     % gspc::installation (vm).gspc_home()
     % (test::source_directory (vm) / "use_sequence.xml")
     % (temporary_path / "use_sequence.pnet.verification")
     ).str()
    );

  std::set<std::string> const expected_result
    { "stdin::_outer_step: (TERMINATES)"
    , "stdin::_outer_not_break: (TERMINATES)"
    , "stdin::_outer_break: (TERMINATES)"
    , "stdin::_outer_init: (TERMINATES)"
    , "stdin::_inner_OUT: (TERMINATES)"
    , "stdin::_inner_IN: (TERMINATES)"
    , "stdin::_inner_step: (TERMINATES)"
    , "stdin::_inner_not_break: (TERMINATES)"
    , "stdin::_inner_break: (TERMINATES)"
    , "stdin::_inner_init: (TERMINATES)"
    , "stdin::_outer_IN: (TERMINATES)"
    , "stdin::_outer_OUT: (TERMINATES)"
    , "stdin: (TERMINATES)"
    };

  auto const lines (fhg::util::read_lines ( temporary_path
                                          / "use_sequence.pnet.verification"
                                          )
                   );

  std::set<std::string> const result {lines.cbegin(), lines.cend()};

   BOOST_REQUIRE_EQUAL_COLLECTIONS
     ( expected_result.cbegin(), expected_result.cend()
     , result.cbegin(), result.cend()
     );
}
