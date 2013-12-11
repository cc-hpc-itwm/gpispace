// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE expr_type_calculate
#include <boost/test/unit_test.hpp>

#include <we/mgmt/bits/signal.hpp>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

namespace
{
  void count (int& counter)
  {
    ++counter;
  }

  void m (std::string& accu, int id, const std::string& s)
  {
    accu += boost::lexical_cast<std::string> (id) + " " + s;
  }
}

BOOST_AUTO_TEST_CASE (sig_count)
{
  int counter (0);

  we::mgmt::util::signal<> sig_cnt;
  sig_cnt.connect (boost::bind (count, boost::ref (counter)));

  BOOST_REQUIRE_EQUAL (counter, 0);
  sig_cnt();
  BOOST_REQUIRE_EQUAL (counter, 1);
  sig_cnt();
  BOOST_REQUIRE_EQUAL (counter, 2);
}

BOOST_AUTO_TEST_CASE (accu)
{
  std::string text;

  we::mgmt::util::signal<void (int, const std::string &)> sig_sig;

  sig_sig.connect (boost::bind (m, boost::ref (text), _1, _2));

  BOOST_REQUIRE_EQUAL (text, "");
  sig_sig (1, "Hallo ");
  BOOST_REQUIRE_EQUAL (text, "1 Hallo ");
  sig_sig (2, "Welt!");
  BOOST_REQUIRE_EQUAL (text, "1 Hallo 2 Welt!");
}
