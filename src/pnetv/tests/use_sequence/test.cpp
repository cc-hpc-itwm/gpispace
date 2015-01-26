// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE pnetv_use_sequence
#include <boost/test/unit_test.hpp>

#include <drts/drts.hpp>

#include <test/make.hpp>
#include <test/source_directory.hpp>

#include <fhg/util/read_file.hpp>

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

  gspc::installation const installation (vm);

  test::make const make
    ( installation
    , "use_sequence"
    , test::source_directory (vm)
    , {{"XML", "use_sequence.xml"}}
    , "verify"
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
     % (make.build_directory() / "use_sequence.pnet").string()
     ).str()
    );

  BOOST_REQUIRE_EQUAL ( expected_result
                      , fhg::util::read_file ( make.build_directory()
                                             / "use_sequence.pnet.verification"
                                             )
                      );
}
