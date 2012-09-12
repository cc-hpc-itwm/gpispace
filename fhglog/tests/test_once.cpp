/*
 * =====================================================================================
 *
 *       Filename:  test_once.cpp
 *
 *    Description:  Tests if the ONCE_* macro works
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

#include <sstream> // ostringstream
#include <vector>

#include <fhglog/fhglog.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/format.hpp>

class VectorAppender : public fhg::log::Appender
{
public:
  typedef std::vector<std::string> container_type;

  VectorAppender(const std::string &a_name, container_type & vec, const std::string & fmt)
    : fhg::log::Appender(a_name)
    , vec_ (vec)
    , fmt_(fmt)
  {}

  void append(const fhg::log::LogEvent &evt)
  {
    vec_.push_back (fhg::log::format(fmt_, evt));
  }

  const container_type & list () const
  {
    return vec_;
  }

  void flush () {}
private:
  container_type & vec_;
  std::string fmt_;
};

static int test_once (const size_t N)
{
  using namespace fhg::log;

  logger_t log(getLogger());
  log.removeAllAppenders();
  log.setLevel (LogLevel::MIN_LEVEL);

  VectorAppender::container_type vec;
  log.addAppender(Appender::ptr_t(new VectorAppender("vectorappender", vec, "%m")));

  {
    std::clog << "** testing appending only once " << N << " events...";
    for (size_t i(0); i < N; ++i)
    {
      FHGLOG_DO_ONCE(LLOG(INFO, log, "iteration #" << i));
    }
    if (vec.size() != 1)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tmessages logged: " << vec.size() << std::endl;
      std::clog << "\texpected: " << 1 << std::endl;
      return 1;
    }
    else
    {
      std::clog << "OK!" << std::endl;
    }
    vec.clear();
  }

  log.removeAllAppenders();
  return 0;
}

int main (int , char **)
{
  using namespace fhg::log;

  int errcount(0);
  logger_t log(getLogger());
  log.removeAllAppenders();
  log.setLevel (LogLevel::MIN_LEVEL);

  VectorAppender::container_type vec;
  log.addAppender(Appender::ptr_t(new VectorAppender("vectorappender", vec, "%m")));

  errcount += test_once (50);

  return errcount;
}
