// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE process_redefine
#include <boost/test/unit_test.hpp>

#include <process.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/test/require_exception.hpp>

BOOST_AUTO_TEST_CASE (process_redefine)
{
  fhg::util::boost::test::require_exception<std::runtime_error>
    ([]
    {
      process::file_const_buffer file (nullptr, 0, "%redefinition%");

      process::file_const_buffer_list files_input;

      files_input.push_back (file);
      files_input.push_back (file);

      process::execute ( std::string()
                       , process::const_buffer (nullptr, 0)
                       , process::buffer (nullptr, 0)
                       , files_input
                       , process::file_buffer_list()
                       );
    }
    , "redefinition of key: %redefinition%"
    );
}
