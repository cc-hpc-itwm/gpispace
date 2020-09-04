// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <boost/test/unit_test.hpp>

#include <we/type/bitsetofint.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <sstream>
#include <set>

BOOST_AUTO_TEST_CASE (various_tests)
{
  bitsetofint::type set(1);

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
    bitsetofint::type a (1); // 64bit
    bitsetofint::type b (2); // 128bit

    a.ins (32);
    b.ins (96);

    bitsetofint::type c = a | b;

    BOOST_REQUIRE (c.is_element(32));
    BOOST_REQUIRE (c.is_element(96));
  }

  {
    bitsetofint::type bs;

    BOOST_REQUIRE_EQUAL (bitsetofint::to_hex (bs), "0x/");

    bs.ins (0);

    BOOST_REQUIRE_EQUAL (bitsetofint::to_hex (bs), "0x/0000000000000001/");

    bs.ins (1);

    BOOST_REQUIRE_EQUAL (bitsetofint::to_hex (bs), "0x/0000000000000003/");

    bs.ins (2);

    BOOST_REQUIRE_EQUAL (bitsetofint::to_hex (bs), "0x/0000000000000007/");

    bs.ins (3);

    BOOST_REQUIRE_EQUAL (bitsetofint::to_hex (bs), "0x/000000000000000f/");

    bs.ins (4);

    BOOST_REQUIRE_EQUAL (bitsetofint::to_hex (bs), "0x/000000000000001f/");

    bs.ins (64);

    BOOST_REQUIRE_EQUAL (bitsetofint::to_hex (bs), "0x/000000000000001f/0000000000000001/");
  }
}

BOOST_AUTO_TEST_CASE (string_conversion)
{
  {
    BOOST_REQUIRE_EQUAL (bitsetofint::from_hex ("0x/"), bitsetofint::type());

    BOOST_REQUIRE_EQUAL ( bitsetofint::from_hex ("0x/0000000000000001/")
                        , bitsetofint::type().ins (0)
                        );

    BOOST_REQUIRE_EQUAL ( bitsetofint::from_hex ("0x/0000000000000003/")
                        , bitsetofint::type().ins (0).ins (1)
                        );

    BOOST_REQUIRE_EQUAL ( bitsetofint::from_hex ("0x/0000000000000007/")
                        , bitsetofint::type().ins (0).ins (1).ins (2)
                        );

    BOOST_REQUIRE_EQUAL ( bitsetofint::from_hex ("0x/000000000000001f/0000000000000001/")
                        , bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64)
                        );
  }

  {
    BOOST_REQUIRE_EQUAL (bitsetofint::from_hex ("0x"), bitsetofint::type());

    BOOST_REQUIRE_EQUAL ( bitsetofint::from_hex ("0x/0000000000000001")
                        , bitsetofint::type().ins (0)
                        );

    BOOST_REQUIRE_EQUAL ( bitsetofint::from_hex ("0x/0000000000000003")
                        , bitsetofint::type().ins (0).ins (1)
                        );

    BOOST_REQUIRE_EQUAL ( bitsetofint::from_hex ("0x/0000000000000007")
                        , bitsetofint::type().ins (0).ins (1).ins (2)
                        );

    BOOST_REQUIRE_EQUAL ( bitsetofint::from_hex ("0x/000000000000001f/0000000000000001")
                        , bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64)
                        );
  }

  {
    std::string inp ("");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (!bs);
    BOOST_REQUIRE_EQUAL (std::string (pos, end), inp);
  }

  {
    std::string inp ("foo");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (!bs);
    BOOST_REQUIRE_EQUAL (std::string (pos, end), inp);
  }

  {
    std::string inp ("0x");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "");
  }

  {
    std::string inp ("0x/");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "");
  }

  {
    std::string inp ("0x/foo");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "foo");
  }

  {
    std::string inp ("0x/0000000000000000");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "");
  }

  {
    std::string inp ("0x/000000000000000");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "000000000000000");
  }

  {
    std::string inp ("0x/0000000000000000/");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, bitsetofint::type());
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "");
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001/");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "");
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001/foo");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "foo");
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001!");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "!");
  }

  {
    std::string inp ("0x/000000000000001f/0000000000000001 foo");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, bitsetofint::type().ins (0).ins (1).ins (2).ins (3).ins (4).ins (64));
    BOOST_REQUIRE_EQUAL (std::string (pos, end), " foo");
  }

  {
    std::string inp ("0x/0000000000000001!0000000000000001/");
    std::string::const_iterator pos (inp.begin());
    const std::string::const_iterator& end (inp.end());

    boost::optional<bitsetofint::type> bs (bitsetofint::from_hex (pos, end));

    BOOST_REQUIRE (bs);
    BOOST_REQUIRE_EQUAL (*bs, bitsetofint::type().ins (0));
    BOOST_REQUIRE_EQUAL (std::string (pos, end), "!0000000000000001/");
  }

  {
    bitsetofint::type bs = bitsetofint::from_hex ("0x/0000000000000000");

    std::size_t      res = bs.count ();

    std::size_t      exp = 0;

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    bitsetofint::type bs = bitsetofint::from_hex ("0x/ffffffffffffffff");

    std::size_t      res = bs.count ();

    std::size_t      exp = 64;

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    bitsetofint::type bs = bitsetofint::from_hex ("0x/f0f0f0f0f0f0f0f0");

    std::size_t      res = bs.count ();

    std::size_t      exp = 8*4;

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    bitsetofint::type bs = bitsetofint::from_hex ("0x/1111111111111111");

    std::size_t      res = bs.count ();

    std::size_t      exp = 16;

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    bitsetofint::type bs = bitsetofint::from_hex ("0x/ffffffffffffffff/1111111111111111");

    std::size_t      res = bs.count ();

    std::size_t      exp = 64 + 16;

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/0000000000000000");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/0000000000000000");

    bitsetofint::type res = bs1 & bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/0000000000000000");

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/1010101010101010");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/0000000000000000");

    bitsetofint::type res = bs1 & bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/0000000000000000");

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/ffffffffffffffff");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/ffffffffffffffff");

    bitsetofint::type res = bs1 & bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/ffffffffffffffff");

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/0000000000000000");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/1111111111111111");

    bitsetofint::type res = bs1 ^ bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/1111111111111111");

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/1010101010101010");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/1010101010101010");

    bitsetofint::type res = bs1 ^ bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/0000000000000000");

    BOOST_REQUIRE_EQUAL (res, exp);
  }

  {
    bitsetofint::type bs1 = bitsetofint::from_hex ("0x/1010101010101010");
    bitsetofint::type bs2 = bitsetofint::from_hex ("0x/0101010101010101");

    bitsetofint::type res = bs1 ^ bs2;

    bitsetofint::type exp = bitsetofint::from_hex ("0x/1111111111111111");

    BOOST_REQUIRE_EQUAL (res, exp);
  }
}

BOOST_AUTO_TEST_CASE (listing)
{
  {
    bitsetofint::type b;

    std::ostringstream s;

    b.list (s);

    BOOST_REQUIRE_EQUAL (s.str(), "");
  }

  {
    bitsetofint::type b (bitsetofint::type().ins (0));

    std::ostringstream s;

    b.list (s);

    BOOST_REQUIRE_EQUAL (s.str(), "0\n");
  }

  {
    bitsetofint::type b (bitsetofint::type().ins (3141));

    std::ostringstream s;

    b.list (s);

    BOOST_REQUIRE_EQUAL (s.str(), "3141\n");
  }

  {
    bitsetofint::type b (bitsetofint::type().ins (0).ins (1).ins (13).ins (42));

    std::ostringstream s;

    b.list (s);

    BOOST_REQUIRE_EQUAL (s.str(), "0\n1\n13\n42\n");
  }

  {
    bitsetofint::type b;
    std::set<std::size_t> s;

    b.list ([&s] (std::size_t x) { s.insert (x); });

    BOOST_REQUIRE_EQUAL (s.size(), 0);
    BOOST_REQUIRE_EQUAL (b.count(), 0);

    const std::set<unsigned long> e (b.elements());
    BOOST_REQUIRE_EQUAL_COLLECTIONS (s.begin(), s.end(), e.begin(), e.end());
  }

  {
    bitsetofint::type b (bitsetofint::type().ins (42));
    std::set<std::size_t> s;

    b.list ([&s] (std::size_t x) { s.insert (x); });

    BOOST_REQUIRE_EQUAL (s.size(), 1);
    BOOST_REQUIRE_EQUAL (b.count(), 1);
    BOOST_REQUIRE_EQUAL (s.count (42), 1);

    const std::set<unsigned long> e (b.elements());
    BOOST_REQUIRE_EQUAL_COLLECTIONS (s.begin(), s.end(), e.begin(), e.end());
  }

  {
    bitsetofint::type b (bitsetofint::type().ins (0).ins (1).ins (13).ins (42));
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
  oss << bitsetofint::type().ins (1).del (1);

  BOOST_REQUIRE_EQUAL (oss.str(), "{}");
}
