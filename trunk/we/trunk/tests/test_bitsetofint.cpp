// mirko.rahn@itwm.fraunhofer.de

#include <we/type/bitsetofint.hpp>

typedef bitsetofint::type set_t;

#include <iostream>
#include <sstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <boost/version.hpp>

#include "timer.hpp"

using std::cout;
using std::cerr;
using std::endl;

#if BOOST_VERSION <= 103800
#  define DISABLE_BINARY_TESTS
#endif

int
main ()
{
  size_t ec (0); // error counter

#if 0
  set_t set(1);

  cout << set;

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

  cout << set;

#ifndef DISABLE_BINARY_TESTS
  {
    std::ostringstream oss;

    boost::archive::binary_oarchive oa (oss, boost::archive::no_header);
    oa << BOOST_SERIALIZATION_NVP(set);

    cout << "size serialize: binary: " << oss.str().length() << endl;

    set_t c;

    {
      std::istringstream iss(oss.str());
      boost::archive::binary_iarchive oa (iss, boost::archive::no_header);
      oa >> BOOST_SERIALIZATION_NVP(c);
    }

    for (unsigned int i (0); i < 80; ++i)
      cout << c.is_element(i);
    cout << endl;
  }
#endif

  {
    std::ostringstream oss;

    boost::archive::text_oarchive oa (oss, boost::archive::no_header);
    oa << BOOST_SERIALIZATION_NVP(set);

    cout << "size serialize: text: " << oss.str().length() << endl;

    set_t c;

    {
      std::istringstream iss(oss.str());
      boost::archive::text_iarchive oa (iss, boost::archive::no_header);
      oa >> BOOST_SERIALIZATION_NVP(c);
    }

    for (unsigned int i (0); i < 80; ++i)
      cout << c.is_element(i);
    cout << endl;
  }

  {
    std::ostringstream oss;

    boost::archive::xml_oarchive oa (oss, boost::archive::no_header);
    oa << BOOST_SERIALIZATION_NVP(set);

    cout << "size serialize: xml: " << oss.str().length() << endl;

    set_t c;

    {
      std::istringstream iss(oss.str());
      boost::archive::xml_iarchive oa (iss, boost::archive::no_header);
      oa >> BOOST_SERIALIZATION_NVP(c);
    }

    for (unsigned int i (0); i < 80; ++i)
      cout << c.is_element(i);
    cout << endl;
  }

  cout << "*** filled up to " << (1 << 20) << endl;

  set.ins (1 << 20);

  cout << set;

  {
    std::ostringstream oss;

    {
      Timer_t timer ("serialize: text");

      boost::archive::text_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP(set);
    }

    cout << "size serialize: text: " << oss.str().length() << endl;

    set_t c;

    {
      Timer_t timer ("deserialize: text");

      std::istringstream iss(oss.str());
      boost::archive::text_iarchive oa (iss, boost::archive::no_header);
      oa >> BOOST_SERIALIZATION_NVP(c);
    }
  }

#ifndef DISABLE_BINARY_TESTS
  {
    std::ostringstream oss;

    {
      Timer_t timer ("serialize: binary");

      boost::archive::binary_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP(set);
    }

    cout << "size serialize: text: " << oss.str().length() << endl;

    set_t c;

    {
      Timer_t timer ("deserialize: binary");

      std::istringstream iss(oss.str());
      boost::archive::binary_iarchive oa (iss, boost::archive::no_header);
      oa >> BOOST_SERIALIZATION_NVP(c);
    }
  }
#endif

  cout << "*** filled up to " << std::numeric_limits<unsigned int>::max() << endl;

  set.ins (std::numeric_limits<unsigned int>::max());

  {
    std::ostringstream oss;

    {
      Timer_t timer ("serialize: text");

      boost::archive::text_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP(set);
    }

    cout << "size serialize: text: " << oss.str().length() << endl;

    set_t c;

    {
      Timer_t timer ("deserialize: text");

      std::istringstream iss(oss.str());
      boost::archive::text_iarchive oa (iss, boost::archive::no_header);
      oa >> BOOST_SERIALIZATION_NVP(c);
    }
  }

#ifndef DISABLE_BINARY_TESTS
  {
    std::ostringstream oss;

    {
      Timer_t timer ("serialize: binary");

      boost::archive::binary_oarchive oa (oss, boost::archive::no_header);
      oa << BOOST_SERIALIZATION_NVP(set);
    }

    cout << "size serialize: text: " << oss.str().length() << endl;

    set_t c;

    {
      Timer_t timer ("deserialize: binary");

      std::istringstream iss(oss.str());
      boost::archive::binary_iarchive oa (iss, boost::archive::no_header);
      oa >> BOOST_SERIALIZATION_NVP(c);
    }
  }
#endif

#endif

  cout << "testing bitwise operations on bitsetofint" << std::endl;

  cout << "*** OR: ";

  {
    set_t a (1); // 64bit
    set_t b (2); // 128bit

    a.ins (32);
    b.ins (96);

    set_t c = a | b;

    if (not c.is_element(32) || not c.is_element(96))
    {
      cout << "FAILED" << endl;
      cerr << "ERROR: expected bits 32 and 96 to be set, but they were not!" << endl;
      cerr << std::hex << "       a := " << a << endl;
      cerr << std::hex << "       b := " << b << endl;
      cerr << std::hex << "   a | b := " << c << endl;
      ++ec;
    }
    else
    {
      cout << "SUCCESS" << endl;
    }
  }

  return EXIT_SUCCESS;
}
