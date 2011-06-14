/*
 * =====================================================================================
 *
 *       Filename:  test_every_n.cpp
 *
 *    Description:  Tests if the EVERY_N macro works
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

template <typename T>
T calc_expected ( T N, T n )
{
  //  echo $(( (50 - (50 % 3 + 1)) / 3 + 1 ))
  //  return ( (N - (N % n + 1)) / n + 1 );
  return N / n;
}

static int test_every_n (const size_t N, const size_t Num = 100)
{
  using namespace fhg::log;
  const size_t expected (calc_expected (Num, N));
  logger_t log(getLogger());
  log.removeAllAppenders();
  log.setLevel (LogLevel::MIN_LEVEL);

  VectorAppender::container_type vec;
  log.addAppender(Appender::ptr_t(new VectorAppender("vectorappender", vec, "%m")));

  {
    std::clog << "** testing appending only every " << N << " events...";
    for (size_t i(0); i < Num; ++i)
    {
      FHGLOG_DO_EVERY_N(N, LLOG(INFO, log, "iteration #" << i));
    }
    if (vec.size() != expected)
    {
      std::clog << "FAILED!" << std::endl;
      std::clog << "\tmessages logged: " << vec.size() << std::endl;
      std::clog << "\texpected: " << expected << std::endl;
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

//   {
//     const size_t N (1);
//     const size_t Num (50);

//     const size_t expected (calc_expected(Num, N));

//     std::clog << "** testing appending only every " << N << " events...";
//     for (size_t i(1); i <= Num; ++i)
//     {
//       FHGLOG_DO_EVERY_N(N, LLOG(INFO, log, "iteration #" << i));
//     }
//     if (vec.size() != expected)
//     {
//       std::clog << "FAILED!" << std::endl;
//       std::clog << "\tmessages logged: " << vec.size() << std::endl;
//       std::clog << "\texpected: " << expected << std::endl;
//       ++errcount;
//     }
//     else
//     {
//       std::clog << "OK!" << std::endl;
//     }
//     vec.clear();
//   }

//   {
//     const size_t N (2);
//     const size_t Num (50);

//     const size_t expected (calc_expected(Num, N));

//     std::clog << "** testing appending only every " << N << " events...";
//     for (size_t i(1); i <= Num; ++i)
//     {
//       FHGLOG_DO_EVERY_N(N, LLOG(INFO, log, "iteration #" << i));
//     }
//     if (vec.size() != expected)
//     {
//       std::clog << "FAILED!" << std::endl;
//       std::clog << "\tmessages logged: " << vec.size() << std::endl;
//       std::clog << "\texpected: " << expected << std::endl;
//       ++errcount;
//     }
//     else
//     {
//       std::clog << "OK!" << std::endl;
//     }
//     vec.clear();
//   }

//   {
//     const size_t N (3);
//     const size_t Num (50);

//     const size_t expected (calc_expected(Num, N));

//     std::clog << "** testing appending only every " << N << " events...";
//     for (size_t i(1); i <= Num; ++i)
//     {
//       FHGLOG_DO_EVERY_N(N, LLOG(INFO, log, "iteration #" << i));
//     }
//     if (vec.size() != expected)
//     {
//       std::clog << "FAILED!" << std::endl;
//       std::clog << "\tmessages logged: " << vec.size() << std::endl;
//       std::clog << "\texpected: " << expected << std::endl;
//       ++errcount;
//     }
//     else
//     {
//       std::clog << "OK!" << std::endl;
//     }
//     vec.clear();
//   }

//   {
//     const size_t N (4);
//     const size_t Num (50);

//     const size_t expected (calc_expected(Num, N));

//     std::clog << "** testing appending only every " << N << " events...";

//     for (size_t i(1); i <= Num; ++i)
//     {
//       FHGLOG_DO_EVERY_N(N, LLOG(INFO, log, "iteration #" << i));
//     }
//     if (vec.size() != expected)
//     {
//       std::clog << "FAILED!" << std::endl;
//       std::clog << "\tmessages logged: " << vec.size() << std::endl;
//       std::clog << "\texpected: " << expected << std::endl;
//       ++errcount;
//     }
//     else
//     {
//       std::clog << "OK!" << std::endl;
//     }
//     vec.clear();
//   }

  for (size_t i = (1); i <= 50; ++i)
  {
    errcount += test_every_n (i, 50);
  }

  return errcount;
}
