// usage of cross, mirko.rahn@itwm.fraunhofer.de

#include <we/util/cross.hpp>

#include <cstdlib>

#include <iostream>
#include <iomanip>

#include <vector>
#include <string>

#include <boost/unordered_map.hpp>
#include <boost/random.hpp>

using std::cout;
using std::endl;
using std::setw;

namespace
{
  typedef std::string key_t;
  typedef int val_t;
  typedef std::vector<val_t> vec_val_t;
  typedef std::pair<key_t,val_t> ret_t;

  typedef boost::unordered_map<key_t,vec_val_t> map_t;
  typedef cross::cross<map_t> cross_t;

  static std::ostream & operator << (std::ostream & s, const ret_t & kv)
  {
    return s << setw(5) << kv.first << ":" << kv.second;
  }

  static void traverse (cross_t c)
  {
    cout << "traverse:" << endl;

    std::size_t k (0);

    for ( ; c.has_more(); ++c)
      {
        cout << setw(2) << k++ << "| ";

        for (cross::iterator<map_t> i (*c); i.has_more(); ++i)
          cout << *i;
        cout << endl;
      }
  }
}

int
main ()
{
  map_t map;

  map["1234"].push_back (1);
  map["1234"].push_back (2);
  map["1234"].push_back (3);
  map["1234"].push_back (4);

  map["56"].push_back (5);
  map["56"].push_back (6);

  map["789"].push_back (7);
  map["789"].push_back (8);
  map["789"].push_back (9);

  traverse (cross_t (map));

  cross::pos_t shift;

  shift.push_back (2);
  shift.push_back (1);
  shift.push_back (3);

  traverse (cross_t (map, shift));

  boost::mt19937 engine (42U);

  traverse (cross_t (map, engine));

  cout << "size = " << cross_t (map).size() << std::endl;

  return EXIT_SUCCESS;
}
