// mirko.rahn@itwm.fraunhofer.de

#include <vector>

typedef std::vector<bool> set_t;

#include <iostream>
#include <sstream>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <boost/serialization/vector.hpp>

#include "timer.hpp"

using std::cout;
using std::endl;

static std::ostream & operator << (std::ostream & s, const set_t & t)
{
  s << "{";
  for (set_t::const_iterator it (t.begin()); it != t.end(); ++it)
    s << ((it != t.begin()) ? "," : "") << *it;
  return s << "}" << std::endl;
}

int
main ()
{
  set_t set;

  cout << set;

  set.resize (80);

  for (unsigned int i (0); i < 80; ++i)
    cout << set[i];
  cout << endl;

  set[13] = true;
  set[22] = true;
  set[69] = true;

  for (unsigned int i (0); i < 80; ++i)
    cout << set[i];
  cout << endl;

  set[13] = true;
  set[22] = false;
  set[70] = true;

  for (unsigned int i (0); i < 80; ++i)
    cout << set[i];
  cout << endl;

  cout << set;

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
      cout << c[i];
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
      cout << c[i];
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
      cout << c[i];
    cout << endl;
  }

  cout << "*** filled up to " << (1 << 20) << endl;

  set.resize (1 << 20);

  set[(1 << 20) - 1] = true;

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

  cout << "*** filled up to " << std::numeric_limits<unsigned short>::max() << endl;

  set.resize (std::numeric_limits<unsigned short>::max());

  set[std::numeric_limits<unsigned short>::max()-1] = true;

  //  cout << set;

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
