#include <iostream>
#include <set>
#include <algorithm>
#include <we/mgmt/type/preference.hpp>
#include <functional>

using namespace we::mgmt::pref;

int main()
{
  preference_t<unsigned int> p(false);

  p.want (5).want (6).want (10);
  p.cant (1).cant (8).cant (3).cant (4);

  std::cout << p << std::endl;
  {
    preference_t<unsigned int> p1 (p);
    std::cout << p1 << std::endl;
  }

  std::vector<unsigned int> workers;
  for (preference_t<unsigned int>::value_type rank(0); rank < 10; ++rank)
  {
    workers.push_back (rank);
  }
  std::cout << "available workers: "
            << util::show (workers.begin (), workers.end())
            << std::endl;

  for ( std::vector<unsigned int>::const_iterator w (workers.begin())
      ; w != workers.end()
      ; ++w
      )
  {
    std::cout << "can (" << *w << ")? " << p.can(*w) << std::endl;
  }

  std::vector<unsigned int>::iterator new_end
    ( std::remove_if ( workers.begin()
                     , workers.end()
                     , std::not1 ( p )
                     )
    );

  std::cout << "possible workers: "
            << util::show (workers.begin (), new_end)
            << std::endl;

  return EXIT_SUCCESS;
}
