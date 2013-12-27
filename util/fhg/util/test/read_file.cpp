// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE read_file
#include <boost/test/unit_test.hpp>

#include <fhg/util/read_file.hpp>

#include <fhg/util/temporary_file.hpp>

#include <fstream>

namespace
{
  void test (std::string const& content)
  {
    boost::filesystem::path const path ("temporary_file");

    BOOST_REQUIRE (!boost::filesystem::exists (path));

    fhg::util::temporary_file const _ (path);

    {
      std::ofstream stream (path.string().c_str());

      stream << content;
    }

    BOOST_REQUIRE_EQUAL (fhg::util::read_file (path.string()), content);
  }
}

BOOST_AUTO_TEST_CASE (read_file)
{
  test ("");
  test ("foo");
  test ("foo\n");
  test ("foo\nbar");
  test ("foo bar");
  test ("foo bar\nbaz faz\n");
}
