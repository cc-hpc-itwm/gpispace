// useage of cross, mirko.rahn@itwm.fraunhofer.de

#include <cross.hpp>

#include <cstdlib>

#include <iostream>
#include <iomanip>

#include <map>
#include <vector>
#include <string>

int
main ()
{
  typedef std::string key_t;
  typedef int val_t;
  typedef std::vector<val_t> vec_val_t;
  typedef std::pair<key_t,val_t> ret_t;
  typedef std::vector<ret_t> cross_t;

  typedef std::map<key_t,vec_val_t> map_t;

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

  while (cross.has_more())
    {
      cross_t c (*cross); ++cross;

      for (cross_t::const_iterator i (c.begin()); i != c.end(); ++i)
        std::cout << std::setw(4) << i->first << ":" << i->second;
      std::cout << std::endl;
    }

  return EXIT_SUCCESS;
}
