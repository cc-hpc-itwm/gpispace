// Copyright (C) 2010-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/we/type/bitsetofint.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <optional>
#include <set>
#include <sstream>

BOOST_AUTO_TEST_CASE (various_tests)
{
  gspc::pnet::type::bitsetofint::type set(1);

  {
    std::ostringstream oss;

    oss << set;

    BOOST_REQUIRE_EQUAL ("{ 0}", oss.str());
  }

  for (unsigned int i (0); i < 80; ++i)
  {
    BOOST_REQUIRE (!set.is_element (i));
  }

  set.ins (13);
  set.ins (22);
  set.ins (69);

  BOOST_REQUIRE (set.is_element (13));
  BOOST_REQUIRE (set.is_element (22));
  BOOST_REQUIRE (set.is_element (69));

  for (unsigned int i (0); i < 80; ++i)
  {
    BOOST_REQUIRE (!set.is_element (i) || i == 13 || i == 22 || i == 69);
  }

  set.ins (13);
  set.del (22);
  set.ins (70);

  BOOST_REQUIRE (set.is_element (13));
  BOOST_REQUIRE (set.is_element (69));
  BOOST_REQUIRE (set.is_element (70));

  for (unsigned int i (0); i < 80; ++i)
  {
    BOOST_REQUIRE (!set.is_element (i) || i == 13 || i == 69 || i == 70);
  }

  {
    std::ostringstream oss;

    oss << set;

    BOOST_REQUIRE_EQUAL ("{ 8192 96}", oss.str());
  }

  set.ins (1 << 20);

  BOOST_REQUIRE_EQUAL (set.count(), 4);
  BOOST_REQUIRE (set.is_element (13));
  BOOST_REQUIRE (set.is_element (69));
  BOOST_REQUIRE (set.is_element (70));
  BOOST_REQUIRE (set.is_element (1 << 20));

  set.ins (std::numeric_limits<unsigned int>::max());

  BOOST_REQUIRE_EQUAL (set.count(), 5);
  BOOST_REQUIRE (set.is_element (13));
  BOOST_REQUIRE (set.is_element (69));
  BOOST_REQUIRE (set.is_element (70));
  BOOST_REQUIRE (set.is_element (1 << 20));
  BOOST_REQUIRE (set.is_element (std::numeric_limits<unsigned int>::max()));
}
BOOST_AUTO_TEST_CASE (bit_operations)
{
  {
    gspc::pnet::type::bitsetofint::type a (1); // 64bit
    gspc::pnet::type::bitsetofint::type b (2); // 128bit

    a.ins (32);
    b.ins (96);

    gspc::pnet::type::bitsetofint::type c = a | b;

    BOOST_REQUIRE (c.is_element(32));
    BOOST_REQUIRE (c.is_element(96));
  }

  {
    gspc::pnet::type::bitsetofint::type bs;

    BOOST_REQUIRE_EQUAL (gspc::pnet::type::bitsetofint::to_hex (bs), "0x/");

    bs.ins (0);

    BOOST_REQUIRE_EQUAL (gspc::pnet::type::bitsetofint::to_hex (bs), "0x/0000000000000001/");

    bs.ins (1);

    BOOST_REQUIRE_EQUAL (gspc::pnet::type::bitsetofint::to_hex (bs), "0x/0000000000000003/");

    bs.ins (2);

    BOOST_REQUIRE_EQUAL (gspc::pnet::type::bitsetofint::to_hex (bs), "0x/0000000000000007/");

    bs.ins (3);

    BOOST_REQUIRE_EQUAL (gspc::pnet::type::bitsetofint::to_hex (bs), "0x/000000000000000f/");

    bs.ins (4);

    BOOST_REQUIRE_EQUAL (gspc::pnet::type::bitsetofint::to_hex (bs), "0x/000000000000001f/");

    bs.ins (64);

    BOOST_REQUIRE_EQUAL (gspc::pnet::type::bitsetofint::to_hex (bs), "0x/000000000000001f/0000000000000001/");
  }
}

BOOST_AUTO_TEST_CASE (string_conversion)
{
  {
    BOOST_REQUIRE_EQUAL (gspc::pnet::type::bitsetofint::from_hex ("0x/"), gspc::pnet::type::bitsetofint::type());

    BOOST_REQUIRE_EQUAL ( gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000001/")
                        , gspc::pnet::type::bitsetofint::type().ins (0)
                        );

    BOOST_REQUIRE_EQUAL ( gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000003/")
                        , gspc::pnet::type::bitsetofint::type().ins (0).ins (1)
                        );

    BOOST_REQUIRE_EQUAL ( gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000007/")
                        , gspc::pnet::type::bitsetofint::type().ins (0).ins (1).ins (2)
                        );

    BOOST_REQUIRE_EQUAL ( gspc::pnet::type::bitsetofint::from_hex ("0x/000000000000001f/0000000000000001/")
                        , gspc::pnet::type::bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64)
                        );
  }

  {
    BOOST_REQUIRE_EQUAL (gspc::pnet::type::bitsetofint::from_hex ("0x"), gspc::pnet::type::bitsetofint::type());

    BOOST_REQUIRE_EQUAL ( gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000001")
                        , gspc::pnet::type::bitsetofint::type().ins (0)
                        );

    BOOST_REQUIRE_EQUAL ( gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000003")
                        , gspc::pnet::type::bitsetofint::type().ins (0).ins (1)
                        );

    BOOST_REQUIRE_EQUAL ( gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000007")
                        , gspc::pnet::type::bitsetofint::type().ins (0).ins (1).ins (2)
                        );

    BOOST_REQUIRE_EQUAL ( gspc::pnet::type::bitsetofint::from_hex ("0x/000000000000001f/0000000000000001")
                        , gspc::pnet::type::bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64)
                        );
  }

  {
    std::string inp ("");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (!bs);
    BOOST_REQUIRE_EQUAL (std::string (pos, end), inp);
  }

  {
    std::string inp ("foo");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (!bs);
    BOOST_REQUIRE_EQUAL (std::string (pos, end), inp);
  }

  {
    std::string inp ("0x");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, gspc::pnet::type::bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "");
  }

  {
    std::string inp ("0x/");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, gspc::pnet::type::bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "");
  }

  {
    std::string inp ("0x/foo");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, gspc::pnet::type::bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "foo");
  }

  {
    std::string inp ("0x/0000000000000000");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, gspc::pnet::type::bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "");
  }

  {
    std::string inp ("0x/000000000000000");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, gspc::pnet::type::bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "000000000000000");
  }

  {
    std::string inp ("0x/0000000000000000/");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, gspc::pnet::type::bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "");
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001/");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, gspc::pnet::type::bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "");
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001/foo");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, gspc::pnet::type::bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "foo");
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001!");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, gspc::pnet::type::bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "!");
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001 foo");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, gspc::pnet::type::bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    BOOST_REQUIRE_EQUAL (std::string (pos, end), " foo");
  }

  {
    std::string inp ("0x/0000000000000001!0000000000000001/");
    std::string::const_iterator pos (inp.begin());
    std::string::const_iterator const& end (inp.end());

    std::optional<gspc::pnet::type::bitsetofint::type> bs (gspc::pnet::type::bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, gspc::pnet::type::bitsetofint::type().ins (0));
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "!0000000000000001/");
  }

  {
    gspc::pnet::type::bitsetofint::type bs = gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000000");

    std::size_t      res = bs.count ();

    std::size_t      exp = 0;

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    gspc::pnet::type::bitsetofint::type bs = gspc::pnet::type::bitsetofint::from_hex ("0x/ffffffffffffffff");

    std::size_t      res = bs.count ();

    std::size_t      exp = 64;

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    gspc::pnet::type::bitsetofint::type bs = gspc::pnet::type::bitsetofint::from_hex ("0x/f0f0f0f0f0f0f0f0");

    std::size_t      res = bs.count ();

    std::size_t      exp = 8UL * 4UL;

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    gspc::pnet::type::bitsetofint::type bs = gspc::pnet::type::bitsetofint::from_hex ("0x/1111111111111111");

    std::size_t      res = bs.count ();

    std::size_t      exp = 16;

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    gspc::pnet::type::bitsetofint::type bs = gspc::pnet::type::bitsetofint::from_hex ("0x/ffffffffffffffff/1111111111111111");

    std::size_t      res = bs.count ();

    std::size_t      exp = 64 + 16;

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    gspc::pnet::type::bitsetofint::type bs1 = gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000000");
    gspc::pnet::type::bitsetofint::type bs2 = gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000000");

    gspc::pnet::type::bitsetofint::type res = bs1 & bs2;

    gspc::pnet::type::bitsetofint::type exp = gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000000");

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    gspc::pnet::type::bitsetofint::type bs1 = gspc::pnet::type::bitsetofint::from_hex ("0x/1010101010101010");
    gspc::pnet::type::bitsetofint::type bs2 = gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000000");

    gspc::pnet::type::bitsetofint::type res = bs1 & bs2;

    gspc::pnet::type::bitsetofint::type exp = gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000000");

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    gspc::pnet::type::bitsetofint::type bs1 = gspc::pnet::type::bitsetofint::from_hex ("0x/ffffffffffffffff");
    gspc::pnet::type::bitsetofint::type bs2 = gspc::pnet::type::bitsetofint::from_hex ("0x/ffffffffffffffff");

    gspc::pnet::type::bitsetofint::type res = bs1 & bs2;

    gspc::pnet::type::bitsetofint::type exp = gspc::pnet::type::bitsetofint::from_hex ("0x/ffffffffffffffff");

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    gspc::pnet::type::bitsetofint::type bs1 = gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000000");
    gspc::pnet::type::bitsetofint::type bs2 = gspc::pnet::type::bitsetofint::from_hex ("0x/1111111111111111");

    gspc::pnet::type::bitsetofint::type res = bs1 ^ bs2;

    gspc::pnet::type::bitsetofint::type exp = gspc::pnet::type::bitsetofint::from_hex ("0x/1111111111111111");

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    gspc::pnet::type::bitsetofint::type bs1 = gspc::pnet::type::bitsetofint::from_hex ("0x/1010101010101010");
    gspc::pnet::type::bitsetofint::type bs2 = gspc::pnet::type::bitsetofint::from_hex ("0x/1010101010101010");

    gspc::pnet::type::bitsetofint::type res = bs1 ^ bs2;

    gspc::pnet::type::bitsetofint::type exp = gspc::pnet::type::bitsetofint::from_hex ("0x/0000000000000000");

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    gspc::pnet::type::bitsetofint::type bs1 = gspc::pnet::type::bitsetofint::from_hex ("0x/1010101010101010");
    gspc::pnet::type::bitsetofint::type bs2 = gspc::pnet::type::bitsetofint::from_hex ("0x/0101010101010101");

    gspc::pnet::type::bitsetofint::type res = bs1 ^ bs2;

    gspc::pnet::type::bitsetofint::type exp = gspc::pnet::type::bitsetofint::from_hex ("0x/1111111111111111");

    BOOST_REQUIRE_EQUAL (res, exp);
  }
}

BOOST_AUTO_TEST_CASE (listing)
{
  {
    gspc::pnet::type::bitsetofint::type b;

    std::ostringstream s;

    b.list (s);

    BOOST_REQUIRE_EQUAL (s.str(), "");
  }

  {
    gspc::pnet::type::bitsetofint::type b (gspc::pnet::type::bitsetofint::type().ins (0));

    std::ostringstream s;

    b.list (s);

    BOOST_REQUIRE_EQUAL (s.str(), "0\n");
  }

  {
    gspc::pnet::type::bitsetofint::type b (gspc::pnet::type::bitsetofint::type().ins (3141));

    std::ostringstream s;

    b.list (s);

    BOOST_REQUIRE_EQUAL (s.str(), "3141\n");
  }

  {
    gspc::pnet::type::bitsetofint::type b (gspc::pnet::type::bitsetofint::type().ins (0).ins (1).ins (13).ins (42));

    std::ostringstream s;

    b.list (s);

    BOOST_REQUIRE_EQUAL (s.str(), "0\n1\n13\n42\n");
  }

  {
    gspc::pnet::type::bitsetofint::type b;
    std::set<std::size_t> s;

    b.list ([&s] (std::size_t x) { s.insert (x); });

    BOOST_REQUIRE_EQUAL (s.size(), 0);
    BOOST_REQUIRE_EQUAL (b.count(), 0);

    const std::set<unsigned long> e (b.elements());
    BOOST_REQUIRE_EQUAL_COLLECTIONS (s.begin(), s.end(), e.begin(), e.end());
  }

  {
    gspc::pnet::type::bitsetofint::type b (gspc::pnet::type::bitsetofint::type().ins (42));
    std::set<std::size_t> s;

    b.list ([&s] (std::size_t x) { s.insert (x); });

    BOOST_REQUIRE_EQUAL (s.size(), 1);
    BOOST_REQUIRE_EQUAL (b.count(), 1);
    BOOST_REQUIRE_EQUAL (s.count (42), 1);

    const std::set<unsigned long> e (b.elements());
    BOOST_REQUIRE_EQUAL_COLLECTIONS (s.begin(), s.end(), e.begin(), e.end());
  }

  {
    gspc::pnet::type::bitsetofint::type b (gspc::pnet::type::bitsetofint::type().ins (0).ins (1).ins (13).ins (42));
    std::set<std::size_t> s;

    b.list ([&s] (std::size_t x) { s.insert (x); });

    BOOST_REQUIRE_EQUAL (s.size(), 4);
    BOOST_REQUIRE_EQUAL (b.count(), 4);
    BOOST_REQUIRE_EQUAL (s.count (0), 1);
    BOOST_REQUIRE_EQUAL (s.count (1), 1);
    BOOST_REQUIRE_EQUAL (s.count (13), 1);
    BOOST_REQUIRE_EQUAL (s.count (42), 1);

    const std::set<unsigned long> e (b.elements());
    BOOST_REQUIRE_EQUAL_COLLECTIONS (s.begin(), s.end(), e.begin(), e.end());
  }
}

BOOST_AUTO_TEST_CASE (trailing_zeros_are_removed)
{
  std::ostringstream oss;
  oss << gspc::pnet::type::bitsetofint::type().ins (1).del (1);

  BOOST_REQUIRE_EQUAL (oss.str(), "{}");
}
