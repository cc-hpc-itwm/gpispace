#include <boost/test/unit_test.hpp>

#include <gspc/util/read_file.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/vector.hpp>
#include <gspc/testing/random.hpp>

#include <fstream>

namespace
{
  void test (std::string const& content)
  {
    std::filesystem::path const path ("temporary_file-read_file");

    BOOST_REQUIRE (!std::filesystem::exists (path));

    gspc::util::temporary_file const _ (path);

    std::ofstream {path} << content;

    BOOST_REQUIRE_EQUAL (gspc::util::read_file (path), content);
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

BOOST_AUTO_TEST_CASE (read_file_as)
{
  std::filesystem::path const path ("temporary_file-read_file");

  BOOST_REQUIRE (!std::filesystem::exists (path));

  gspc::util::temporary_file const _ (path);

  std::ofstream {path} << path;

  BOOST_REQUIRE_EQUAL
    (gspc::util::read_file_as<std::filesystem::path> (path), path);
}

BOOST_AUTO_TEST_CASE (read_into_vector_of_char)
{
  std::filesystem::path const path ("temporary_file-read_file");

  BOOST_REQUIRE (!std::filesystem::exists (path));

  gspc::util::temporary_file const _ (path);

  std::vector<char> const content
    {gspc::testing::randoms<std::vector<char>> (1 << 20)};

  std::ofstream {path} << std::string (content.begin(), content.end());

  std::vector<char> const read {gspc::util::read_file<std::vector<char>> (path)};

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (content.begin(), content.end(), read.begin(), read.end());
}
