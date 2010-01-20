// useage of cross, mirko.rahn@itwm.fraunhofer.de

#include <cross.hpp>

#include <cstdlib>

#include <iostream>

#include <map>
#include <vector>
#include <string>

int
main ()
{
  typedef std::map<std::string,std::vector<int> > map_t;

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
      std::vector<int> c (*cross); ++cross;

      for (std::vector<int>::const_iterator i (c.begin()); i != c.end(); ++i)
        std::cout << " " << *i;
      std::cout << std::endl;
    }

  return EXIT_SUCCESS;
}
