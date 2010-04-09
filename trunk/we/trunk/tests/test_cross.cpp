// usage of cross, mirko.rahn@itwm.fraunhofer.de

#include <we/util/cross.hpp>

#include <cstdlib>

#include <iostream>
#include <iomanip>

#include <vector>
#include <string>

#include <boost/unordered_map.hpp>

using std::cout;
using std::endl;
using std::setw;

namespace
{
  typedef std::string key_t;
  typedef int val_t;
  typedef std::vector<val_t> vec_val_t;
  typedef std::pair<key_t,val_t> ret_t;
  typedef std::vector<ret_t> cross_t;

  typedef boost::unordered_map<key_t,vec_val_t> map_t;

  static std::ostream & operator << (std::ostream & s, const ret_t & kv)
  {
    return s << setw(4) << kv.first << ":" << kv.second;
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

  cross::cross<map_t> cross (map);

  {
    // traversing as vectors, incremental

    std::size_t k (0);

    for ( ; cross.has_more(); ++cross)
      {
        cout << setw(2) << k++ << "| ";

        cross_t c (cross.get_vec());

        for (cross_t::const_iterator i (c.begin()); i != c.end(); ++i)
          cout << *i;
        cout << endl;
      }
  }

  cout << "size = " << cross.size() << endl;

  cross.rewind();

  {
    // traversing as vectors, get iterators for the entry too

    std::size_t k (0);

    for ( ; cross.has_more(); ++cross)
      {
        cout << setw(2) << k++ << "| ";

        for (cross::star_iterator<map_t> i (*cross); i.has_more(); ++i)
          cout << *i;
        cout << endl;
      }
  }

  {
    // get iterator, explicitely specify the elements

    cross::pos_t p;

    p.push_back (3);
    p.push_back (0);
    p.push_back (1);

    // should equal to cross[11]
    for (cross::star_iterator<map_t> i (cross.by(p)); i.has_more(); ++i)
      cout << *i;
    cout << endl;
  }

  return EXIT_SUCCESS;
}
