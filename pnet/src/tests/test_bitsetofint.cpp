// mirko.rahn@itwm.fraunhofer.de

#include <we/type/bitsetofint.hpp>

typedef bitsetofint::type set_t;

#include <iostream>
#include <sstream>
#include <set>

#include <boost/bind.hpp>

#include "timer.hpp"

using std::cout;
using std::cerr;
using std::endl;

#define REQUIRE(b) if (!(b)) { cerr << "FAILURE in line " << __LINE__ << endl; ++ec; }

namespace
{
  void set_insert (std::set<std::size_t>& s, const std::size_t& x)
  {
    s.insert (x);
  }
}

int
main ()
{
  size_t ec (0); // error counter

  set_t set(1);

  cout << set << endl;

  for (unsigned int i (0); i < 80; ++i)
    cout << set.is_element(i);
  cout << endl;

  set.ins (13);
  set.ins (22);
  set.ins (69);

  for (unsigned int i (0); i < 80; ++i)
    cout << set.is_element(i);
  cout << endl;

  set.ins (13);
  set.del (22);
  set.ins (70);

  for (unsigned int i (0); i < 80; ++i)
    cout << set.is_element(i);
  cout << endl;

  cout << set << endl;

  cout << "*** filled up to " << (1 << 20) << endl;

  set.ins (1 << 20);

  cout << "*** filled up to " << std::numeric_limits<unsigned int>::max() << endl;

  set.ins (std::numeric_limits<unsigned int>::max());

  cout << "*** bit operations" << endl;

  {
    set_t a (1); // 64bit
    set_t b (2); // 128bit

    a.ins (32);
    b.ins (96);

    set_t c = a | b;

    REQUIRE (c.is_element(32));
    REQUIRE (c.is_element(96));
  }

  {
    set_t bs;

    REQUIRE (bitsetofint::to_hex (bs) == "0x/");

    bs.ins (0);

    REQUIRE (bitsetofint::to_hex (bs) == "0x/0000000000000001/");

    bs.ins (1);

    REQUIRE (bitsetofint::to_hex (bs) == "0x/0000000000000003/");

    bs.ins (2);

    REQUIRE (bitsetofint::to_hex (bs) == "0x/0000000000000007/");

    bs.ins (3);

    REQUIRE (bitsetofint::to_hex (bs) == "0x/000000000000000f/");

    bs.ins (4);

    REQUIRE (bitsetofint::to_hex (bs) == "0x/000000000000001f/");

    bs.ins (64);

    REQUIRE (bitsetofint::to_hex (bs) == "0x/000000000000001f/0000000000000001/");
  }

  cout << "string conversion" << endl;

  {
    REQUIRE (bitsetofint::from_hex ("0x/") == bitsetofint::type());

    REQUIRE (  bitsetofint::from_hex ("0x/0000000000000001/")
            == bitsetofint::type().ins (0)
            );

    REQUIRE (  bitsetofint::from_hex ("0x/0000000000000003/")
            == bitsetofint::type().ins (0).ins (1)
            );

    REQUIRE (  bitsetofint::from_hex ("0x/0000000000000007/")
            == bitsetofint::type().ins (0).ins (1).ins (2)
            );

    REQUIRE (  bitsetofint::from_hex ("0x/000000000000001f/0000000000000001/")
            == bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64)
            );
  }

  {
    REQUIRE (bitsetofint::from_hex ("0x") == bitsetofint::type());

    REQUIRE (  bitsetofint::from_hex ("0x/0000000000000001")
            == bitsetofint::type().ins (0)
            );

    REQUIRE (  bitsetofint::from_hex ("0x/0000000000000003")
            == bitsetofint::type().ins (0).ins (1)
            );

    REQUIRE (  bitsetofint::from_hex ("0x/0000000000000007")
            == bitsetofint::type().ins (0).ins (1).ins (2)
            );

    REQUIRE (  bitsetofint::from_hex ("0x/000000000000001f/0000000000000001")
            == bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64)
            );
  }

  {
    std::string inp ("");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (!bs);
    REQUIRE (pos == inp.begin());
  }

  {
    std::string inp ("foo");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (!bs);
    REQUIRE (pos == inp.begin());
  }

  {
    std::string inp ("0x");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (bs);
    REQUIRE (*bs == bitsetofint::type());
    REQUIRE (pos == end);
  }

  {
    std::string inp ("0x/");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (bs);
    REQUIRE (*bs == bitsetofint::type());
    REQUIRE (pos == end);
  }

  {
    std::string inp ("0x/foo");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (bs);
    REQUIRE (*bs == bitsetofint::type());
    REQUIRE (std::string (pos, end) == "foo");
  }

  {
    std::string inp ("0x/0000000000000000");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (bs);
    REQUIRE (*bs == bitsetofint::type());
    REQUIRE (pos == end);
  }

  {
    std::string inp ("0x/000000000000000");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (bs);
    REQUIRE (*bs == bitsetofint::type());
    REQUIRE (std::string (pos, end) == "000000000000000");
  }

  {
    std::string inp ("0x/0000000000000000/");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (bs);
    REQUIRE (*bs == bitsetofint::type());
    REQUIRE (pos == end);
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001/");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (bs);
    REQUIRE (*bs == bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    REQUIRE (pos == end);
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001/foo");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (bs);
    REQUIRE (*bs == bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    REQUIRE (std::string (pos, end) == "foo");
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001!");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (bs);
    REQUIRE (*bs == bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    REQUIRE (std::string (pos, end) == "!");
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001 foo");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (bs);
    REQUIRE (*bs == bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    REQUIRE (std::string (pos, end) == " foo");
  }

  {
    std::string inp ("0x/0000000000000001!0000000000000001/");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    REQUIRE (bs);
    REQUIRE (*bs == bitsetofint::type().ins (0));
    REQUIRE (std::string (pos, end) == "!0000000000000001/");
  }

  {
    bitsetofint::type bs = bitsetofint::from_hex ("0x/0000000000000000");

    std::size_t      res = bs.count ();

    std::size_t      exp = 0;

    REQUIRE (res == exp);
  }

  {
    bitsetofint::type bs = bitsetofint::from_hex ("0x/ffffffffffffffff");

    std::size_t      res = bs.count ();

    std::size_t      exp = 64;

    REQUIRE (res == exp);
  }

  {
    bitsetofint::type bs = bitsetofint::from_hex ("0x/f0f0f0f0f0f0f0f0");

    std::size_t      res = bs.count ();

    std::size_t      exp = 8*4;

    REQUIRE (res == exp);
  }

  {
    bitsetofint::type bs = bitsetofint::from_hex ("0x/1111111111111111");

    std::size_t      res = bs.count ();

    std::size_t      exp = 16;

    REQUIRE (res == exp);
  }

  {
    bitsetofint::type bs = bitsetofint::from_hex ("0x/ffffffffffffffff/1111111111111111");

    std::size_t      res = bs.count ();

    std::size_t      exp = 64 + 16;

    REQUIRE (res == exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/0000000000000000");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/0000000000000000");

    bitsetofint::type res = bs1 & bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/0000000000000000");

    REQUIRE (res == exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/1010101010101010");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/0000000000000000");

    bitsetofint::type res = bs1 & bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/0000000000000000");

    REQUIRE (res == exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/ffffffffffffffff");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/ffffffffffffffff");

    bitsetofint::type res = bs1 & bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/ffffffffffffffff");

    REQUIRE (res == exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/0000000000000000");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/1111111111111111");

    bitsetofint::type res = bs1 ^ bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/1111111111111111");

    REQUIRE (res == exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/1010101010101010");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/1010101010101010");

    bitsetofint::type res = bs1 ^ bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/0000000000000000");

    REQUIRE (res == exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/1010101010101010");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/0101010101010101");

    bitsetofint::type res = bs1 ^ bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/1111111111111111");

    REQUIRE (res == exp);
  }

  cout << "*** listing" << endl;

  {
    bitsetofint::type b;

    std::ostringstream s;

    b.list (s);

    REQUIRE (s.str() == "");
  }

  {
    bitsetofint::type b (bitsetofint::type().ins (0));

    std::ostringstream s;

    b.list (s);

    REQUIRE (s.str() == "0\n");
  }

  {
    bitsetofint::type b (bitsetofint::type().ins (3141));

    std::ostringstream s;

    b.list (s);

    REQUIRE (s.str() == "3141\n");
  }

  {
    bitsetofint::type b (bitsetofint::type().ins (0).ins (1).ins (13).ins (42));

    std::ostringstream s;

    b.list (s);

    REQUIRE (s.str() == "0\n1\n13\n42\n");
  }

  {
    bitsetofint::type b;
    std::set<std::size_t> s;

    b.list (boost::bind (&set_insert, boost::ref(s), _1));

    REQUIRE (s.size() == 0);
    REQUIRE (s == b.elements());
  }

  {
    bitsetofint::type b (bitsetofint::type().ins (42));
    std::set<std::size_t> s;

    b.list (boost::bind (&set_insert, boost::ref(s), _1));

    REQUIRE (s.size() == 1);
    REQUIRE (s.count (42) == 1);
    REQUIRE (s == b.elements());
  }

  {
    bitsetofint::type b (bitsetofint::type().ins (0).ins (1).ins (13).ins (42));
    std::set<std::size_t> s;

    b.list (boost::bind (&set_insert, boost::ref(s), _1));

    REQUIRE (s.size() == 4);
    REQUIRE (s.count (0) == 1);
    REQUIRE (s.count (1) == 1);
    REQUIRE (s.count (13) == 1);
    REQUIRE (s.count (42) == 1);
    REQUIRE (s == b.elements());
  }

  return ec;
}
