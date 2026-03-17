#include <boost/test/unit_test.hpp>

#include <gspc/util/temporary_file.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <filesystem>
#include <fstream>

BOOST_AUTO_TEST_CASE (temporary_file)
{
  std::filesystem::path const path ("temporary_file-temporary_file");

  BOOST_REQUIRE (!std::filesystem::exists (path));

  {
    gspc::util::temporary_file const _ (path);

    std::ofstream {path};

    BOOST_REQUIRE (std::filesystem::exists (path));
  }

  BOOST_REQUIRE (!std::filesystem::exists (path));
}
