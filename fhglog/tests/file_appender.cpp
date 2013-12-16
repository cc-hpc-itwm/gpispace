// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE file_appender
#include <boost/test/unit_test.hpp>

#include <fhglog/fhglog.hpp>
#include <fhglog/FileAppender.hpp>

#include <fstream> // ifstream

#include <boost/filesystem.hpp>

namespace
{
  std::string content_of_file (const std::string file)
  {
    std::ifstream ifs (file.c_str());
    std::string content;
    std::getline (ifs, content);
    return content;
  }

  struct remove_on_scope_exit
  {
    remove_on_scope_exit (boost::filesystem::path filename)
      : _filename (filename)
    {}
    ~remove_on_scope_exit()
    {
      boost::filesystem::remove (_filename);
    }
  private:
    const boost::filesystem::path _filename;
  };
}

BOOST_AUTO_TEST_CASE (throw_on_unwritable_file)
{
  BOOST_REQUIRE_THROW
    ( fhg::log::FileAppender ("no-perm", "/non-existing-dir/test.log")
    , std::exception
    );
}

BOOST_AUTO_TEST_CASE (remaining)
{
  const remove_on_scope_exit delete_logfile ("test_file_appender.cpp.log");

  fhg::log::FileAppender appender ("logfile", "test_file_appender.cpp.log", "%m");

  appender.append (FHGLOG_MKEVENT_HERE (ERROR, "hello world!"));
  appender.flush();

  BOOST_REQUIRE_EQUAL
    (content_of_file ("test_file_appender.cpp.log"), "hello world!");
}

BOOST_AUTO_TEST_CASE (reopen)
{
  const remove_on_scope_exit delete_logfile ("test_file_appender.cpp.log");

  fhg::log::FileAppender appender ("logfile", "test_file_appender.cpp.log", "%m");

  appender.append (FHGLOG_MKEVENT_HERE (ERROR, "hello world!"));
  appender.flush();

  appender.reopen();
  appender.append (FHGLOG_MKEVENT_HERE (ERROR, "hello world!"));
  appender.flush();

  BOOST_REQUIRE_EQUAL
    (content_of_file ("test_file_appender.cpp.log"), "hello world!hello world!");
}

BOOST_AUTO_TEST_CASE (reopen_with_different_path)
{
  const remove_on_scope_exit delete_foo ("foo.log");
  const remove_on_scope_exit delete_bar ("bar.log");

  fhg::log::FileAppender appender ("appender-to-foo", "foo.log", "%m");

  {
    std::ifstream ifs ("foo.log");
    BOOST_REQUIRE (ifs.is_open());
    BOOST_REQUIRE (ifs.good());
  }

  appender.set_path ("bar.log");
  appender.reopen();
  appender.append (FHGLOG_MKEVENT_HERE (ERROR, "hello world!"));
  appender.flush();

  BOOST_REQUIRE_EQUAL (content_of_file ("bar.log"), "hello world!");
}
