// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE process_redefine
#include <boost/test/unit_test.hpp>

#include <process.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

namespace
{
  //! \todo use boost::bind below
  struct _
  {
    void operator() () const
    {
      process::file_const_buffer file (NULL, 0, "%redefinition%");

      process::file_const_buffer_list files_input;

      files_input.push_back (file);
      files_input.push_back (file);

      process::execute ( std::string()
                       , process::const_buffer (NULL, 0)
                       , process::buffer (NULL, 0)
                       , files_input
                       , process::file_buffer_list()
                       );
    }
  };
}

BOOST_AUTO_TEST_CASE (process_redefine)
{
  fhg::util::boost::test::require_exception<std::runtime_error>
    (_(), "redefinition of key: %redefinition%");
}
