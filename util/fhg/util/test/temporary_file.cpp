// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE temporary_file
#include <boost/test/unit_test.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/temporary_file.hpp>

#include <boost/filesystem.hpp>

#include <fstream>

BOOST_AUTO_TEST_CASE (temporary_file)
{
  boost::filesystem::path const path ("temporary_file-temporary_file");

  BOOST_REQUIRE (!boost::filesystem::exists (path));

  {
    fhg::util::temporary_file const _ (path);

    std::ofstream const __ (path.string().c_str());

    BOOST_REQUIRE (boost::filesystem::exists (path));
  }

  BOOST_REQUIRE (!boost::filesystem::exists (path));
}
