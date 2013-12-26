// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE appender_call
#include <boost/test/unit_test.hpp>

#include <fhglog/appender/call.hpp>
#include <fhglog/LogMacros.hpp>

#include <sstream>

#include <boost/bind.hpp>

namespace
{
  class state_t
  {
  public:
    state_t (std::ostringstream& os)
      : _id (0)
      , _os (os)
    {}
    void fun (fhg::log::LogEvent const& event)
    {
      ++_id;
      _os << event.message() << "\n";
    }
    int id() const
    {
      return _id;
    }
    std::ostringstream& os() const
    {
      return _os;
    }
  private:
    int _id;
    std::ostringstream& _os;
  };
}

BOOST_AUTO_TEST_CASE (call)
{
  std::ostringstream oss;
  std::ostringstream cmp;
  state_t s (oss);
  fhg::log::appender::call c (boost::bind (&state_t::fun, boost::ref (s), _1));

  for (int i (0); i < 10; ++i)
  {
    std::string const msg ("foo");

    BOOST_REQUIRE_EQUAL (s.id(), i);

    c.append (FHGLOG_MKEVENT_HERE (TRACE, msg));

    cmp << msg << "\n";
  }

  BOOST_REQUIRE_EQUAL (cmp.str(), s.os().str());
}
