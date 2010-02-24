// mirko.rahn@itwm.fraunhofer.de

#include <we/util/bitsetofint.hpp>

typedef bitsetofint::type<unsigned int> set_t;

#include <iostream>
#include <sstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include "timer.hpp"

using std::cout;
using std::endl;

int
main ()
{
  set_t set(1);

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

  return EXIT_SUCCESS;
}
