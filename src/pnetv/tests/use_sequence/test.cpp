// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnetv_use_sequence
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <test/source_directory.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/system_with_blocked_SIGCHLD.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

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

  std::string const expected_result
    (( boost::format (R"EOS(%1%::_outer_step: (TERMINATES)
%1%::_outer_not_break: (TERMINATES)
%1%::_outer_OUT: (TERMINATES)
%1%::_inner_init: (TERMINATES)
%1%::_outer_IN: (TERMINATES)
%1%::_inner_break: (TERMINATES)
%1%::_inner_not_break: (TERMINATES)
%1%::_inner_step: (TERMINATES)
%1%::_inner_IN: (TERMINATES)
%1%::_inner_OUT: (TERMINATES)
%1%::_outer_init: (TERMINATES)
%1%::_outer_break: (TERMINATES)
%1%: (TERMINATES)
)EOS")
     % "stdin"
     ).str()
    );

  BOOST_REQUIRE_EQUAL ( expected_result
                      , fhg::util::read_file ( temporary_path
                                             / "use_sequence.pnet.verification"
                                             )
                      );
}
