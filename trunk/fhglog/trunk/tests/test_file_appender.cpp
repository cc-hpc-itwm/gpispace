/*
 * =====================================================================================
 *
 *       Filename:  test_file_appender.cpp
 *
 *    Description:  Tests the file appender for the fhglog logger
 *
 *        Version:  1.0
 *        Created:  09/18/2009 04:38:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <fhglog/fhglog.hpp>
#include <fhglog/FileAppender.hpp>

#include <unistd.h> // unlink
#include <fstream> // ifstream

int main (int , char **)
{
  using namespace fhg::log;

  int errcount(0);
  logger_t log(getLogger());
  log.setLevel(LogLevel::MIN_LEVEL);

  FileAppender::ptr_t file_app(new FileAppender("logfile"
                                              , "test_file_appender.cpp.log"
                                               , "%m"));
  log.addAppender(file_app);

  {
    std::clog << "** testing file appender (simple append)...";
    log.log(FHGLOG_MKEVENT_HERE(DEBUG, "hello world!"));
    file_app->flush();

    const std::string expected("hello world!");
    std::ifstream ifs("test_file_appender.cpp.log");
    std::string actual;
    std::getline(ifs, actual);

    if (expected != actual)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tlogged message: " << actual << std::endl;
      std::clog << "\texpected: " << expected << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
  }

  {
    std::clog << "** testing file appender creation with file-open error...";
    try
    {
      FileAppender::ptr_t app(new FileAppender("no-perm", "/non-existing-dir/test.log"));
      std::clog << "FAILED!" << std::endl;
      std::clog << "\topening '/non-existing-dir/test.log' for writing did not fail!" << std::endl;
      std::clog << "\texpected an exception" << std::endl;
      ++errcount;
    }
    catch (const std::exception &ex)
    {
      std::clog << "OK!" << std::endl;
    }
  }

  {
    std::clog << "** testing reopening file appender with same path...";
    try
    {
      file_app->reopen();
    }
    catch (const std::exception &ex)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\treopening should not fail!" << std::endl;
      ++errcount;
    }

    const std::string expected("hello world!");
    std::ifstream ifs("test_file_appender.cpp.log");
    std::string actual;
    std::getline(ifs, actual);

    if (expected != actual)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tfile content after reopen: " << actual << std::endl;
      std::clog << "\texpected: " << expected << std::endl;
      ++errcount;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
  }

  {
    std::clog << "** testing reopening file appender with different path...";
    try
    {
      FileAppender::ptr_t foo(new FileAppender("appender-to-foo", "foo.log", "%m"));

      {
        std::ifstream ifs("foo.log");
        if ( ! (ifs.is_open() && ifs.good()) )
        {
          std::clog << "FAILED!" << std::endl;
          std::clog << "\topening 'foo.log' for writing failed!" << std::endl;
          ++errcount;
        }
      }

      foo->set_path("bar.log");
      foo->reopen();
      foo->append(FHGLOG_MKEVENT_HERE(DEBUG, "hello world!"));
      foo->flush();

      {
        const std::string expected("hello world!");
        std::ifstream ifs("bar.log");
        std::string actual;
        std::getline(ifs, actual);

        if (expected != actual)
        {
          std::clog << "FAILED!" << std::endl;
          std::clog << "\tbar.log contained: " << actual << std::endl;
          std::clog << "\texpected: " << expected << std::endl;
          ++errcount;
        }
        else
        {
          std::clog << "OK!" << std::endl;
        }
      }

      unlink("foo.log");
      unlink("bar.log");
    }
    catch (const std::exception &ex)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\treopen failed!" << std::endl;
      ++errcount;
    }
  }

  file_app->close();
  unlink("test_file_appender.cpp.log");

  return errcount;
}
