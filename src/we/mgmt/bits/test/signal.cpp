// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE expr_type_calculate
#include <boost/test/unit_test.hpp>

#include <we/mgmt/bits/signal.hpp>
#include <iostream>

static int counter = 0;
static std::string text;

namespace
{
  void count (void)
  {
    std::cerr << "counting" << std::endl;

    counter ++;
  }

  void m (int id, const std::string & s)
  {
    std::cerr << "got signal: " << id << ": " << s << std::endl;
    text += s;
  }
}

BOOST_AUTO_TEST_CASE (sig_count)
{
  we::mgmt::util::signal<> sig_cnt;
  sig_cnt.connect ( &count );

  sig_cnt();
  sig_cnt();

  BOOST_REQUIRE_EQUAL (counter, 2);
}

BOOST_AUTO_TEST_CASE (accu)
{
  we::mgmt::util::signal<void (int, const std::string &)> sig_sig;
  text = "";

  sig_sig.connect ( &m );
  sig_sig ( 1, "Hallo " );
  sig_sig ( 2, "Welt!" );

  BOOST_REQUIRE_EQUAL (text, "Hallo Welt!");
}
