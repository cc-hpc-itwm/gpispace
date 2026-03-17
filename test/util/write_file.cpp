#include <boost/test/unit_test.hpp>

#include <gspc/util/ostream/callback/print.hpp>
#include <gspc/util/read_file.hpp>
#include <gspc/util/temporary_file.hpp>
#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/temporary_path.hpp>
#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_exception.hpp>
#include <gspc/testing/test_case.hpp>
#include <gspc/testing/unique_path.hpp>
#include <gspc/util/write_file.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <fmt/core.h>
#include <numeric>

namespace
{
  template<typename T> std::string as_written (T const& content)
  {
    return gspc::util::ostream::callback::print<T>
      (gspc::util::ostream::callback::id<T>(), content).string();
  }
}

GSPC_TESTING_TEMPLATED_CASE (write_file_stores_content, int, std::string)
{
  gspc::testing::temporary_path const path;
  gspc::util::temporary_file const file
    ( std::filesystem::path {path}
    / gspc::testing::unique_path()
    );

  auto const content (gspc::testing::random<T>{}());

  gspc::util::write_file (file, content);

  BOOST_REQUIRE_EQUAL (gspc::util::read_file (file), as_written (content));
}

BOOST_AUTO_TEST_CASE (write_file_overwrites)
{
  gspc::testing::temporary_path const path;
  gspc::util::temporary_file const file
    ( std::filesystem::path {path}
    / gspc::testing::unique_path()
    );

  std::string const base {gspc::testing::random<std::string>()()};

  gspc::util::write_file (file, base + "1");
  gspc::util::write_file (file, base + "2");

  BOOST_REQUIRE_EQUAL (gspc::util::read_file (file), base + "2");
}

BOOST_AUTO_TEST_CASE (write_file_throws_when_file_can_not_opened)
{
  std::filesystem::path const file ("/");

  gspc::testing::require_exception
    ( [&file]()
      {
        gspc::util::write_file (file, gspc::testing::random<int>()());
      }
    , std::runtime_error {fmt::format ("Could not open {} for writing.", file)}
    );
}

namespace
{
  class throwing_ostream_modifier : public gspc::util::ostream::modifier
  {
  public:
    throwing_ostream_modifier (std::string const& message)
      : _message (message)
    {}
    virtual std::ostream& operator() (std::ostream&) const override
    {
      throw std::logic_error (_message);
    }

  private:
    std::string const _message;
  };

  std::string random_cstr_comparable()
  {
    return gspc::testing::random<std::string>{}
      (gspc::testing::random<char>::any_without_zero());
  }
}

BOOST_AUTO_TEST_CASE (write_file_throws_on_output_failure)
{
  gspc::testing::temporary_path const path;
  gspc::util::temporary_file const file
    ( std::filesystem::path {path}
    / gspc::testing::unique_path()
    );

  std::string const message {random_cstr_comparable()};

  gspc::testing::require_exception
    ( [&file, &message]()
      {
        gspc::util::write_file (file, throwing_ostream_modifier (message));
      }
    , std::logic_error (message)
    );

  BOOST_REQUIRE (std::filesystem::exists (file));
}

namespace
{
  class file_removing_ostream_modifier : public gspc::util::ostream::modifier
  {
  public:
    file_removing_ostream_modifier (std::filesystem::path const& file)
      : _file (file)
    {}
    virtual std::ostream& operator() (std::ostream& os) const override
    {
      std::filesystem::remove (_file);

      return os << gspc::testing::random<std::string>()();
    }

  private:
    std::filesystem::path const _file;
  };
}

BOOST_AUTO_TEST_CASE (write_file_has_file_open_when_doing_output)
{
  gspc::testing::temporary_path const path;
  gspc::util::temporary_file const _
    ( std::filesystem::path {path}
    / gspc::testing::unique_path()
    );
  std::filesystem::path const file (_);

  gspc::util::write_file (file, file_removing_ostream_modifier (file));

  BOOST_REQUIRE (!std::filesystem::exists (file));
}

//! \todo how to test write errors? disk full?
