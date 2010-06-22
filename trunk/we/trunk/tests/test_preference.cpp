#include <iostream>
#include <set>
#include <algorithm>
#include <we/mgmt/type/preference.hpp>
#include <functional>

using namespace we::mgmt::pref;

int main()
{
  preference_t<unsigned int> p (mk_pref<unsigned int> (preferred_tag()));

  p.want (1).want (0).want (2);
  p.cant (9).cant (7).cant (3).cant (4);

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

  std::vector<unsigned int>::iterator bound
    ( std::stable_partition ( workers.begin()
                            , workers.end()
                            , p
                            )
    );

  std::cout << "possible workers: "
            << util::show (workers.begin (), bound)
            << std::endl;
  std::cout << "impossible workers: "
            << util::show (bound, workers.end())
            << std::endl;

  return EXIT_SUCCESS;
}
