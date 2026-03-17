#include <boost/test/unit_test.hpp>

#include <gspc/util/read_file.hpp>
#include <gspc/util/scoped_file_with_content.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/unique_path.hpp>

BOOST_AUTO_TEST_CASE (scoped_file_with_content)
{
  gspc::testing::temporary_path const temporary_directory;

  std::filesystem::path const path
    { std::filesystem::path {temporary_directory}
    / gspc::testing::unique_path()
    };

  BOOST_REQUIRE (!std::filesystem::exists (path));

  {
    std::string const content {gspc::testing::random<std::string>{}()};

    gspc::util::scoped_file_with_content const _ (path, content);

    BOOST_REQUIRE_EQUAL (gspc::util::read_file (path), content);
  }

  BOOST_REQUIRE (!std::filesystem::exists (path));
}
