// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE file_appender
#include <boost/test/unit_test.hpp>

#include <fhglog/fhglog.hpp>
#include <fhglog/appender/file.hpp>

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
    (fhg::log::FileAppender ( "/non-existing-dir/test.log"
                            ,  fhg::log::default_format::LONG()
                            )
    , std::exception
    );
}

BOOST_AUTO_TEST_CASE (remaining)
{
  const remove_on_scope_exit delete_logfile ("test_file_appender.cpp.log");

  fhg::log::FileAppender appender ("test_file_appender.cpp.log", "%m");

  appender.append (FHGLOG_MKEVENT_HERE (ERROR, "hello world!"));
  appender.flush();

  BOOST_REQUIRE_EQUAL
    (content_of_file ("test_file_appender.cpp.log"), "hello world!");
}
