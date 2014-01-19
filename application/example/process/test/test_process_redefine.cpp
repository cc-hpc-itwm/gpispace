// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE process_redefine
#include <boost/test/unit_test.hpp>

#include <process.hpp>

BOOST_AUTO_TEST_CASE (process_redefine)
{
  process::file_const_buffer file (NULL, 0, "%redefinition%");

  process::file_const_buffer_list files_input;

  files_input.push_back (file);
  files_input.push_back (file);

  try
    {
      process::execute ( std::string()
                       , process::const_buffer (NULL,0)
                       , process::buffer (NULL,0)
                       , files_input
                       , process::file_buffer_list ()
                       );
    }
  catch (const std::runtime_error & e)
    {
      BOOST_REQUIRE_EQUAL
        ( std::string (e.what())
        , std::string ("redefinition of key: %redefinition%")
        );
    }
}
