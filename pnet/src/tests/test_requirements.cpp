#include <iostream>
#include <set>
#include <algorithm>
#include <we/type/requirement.hpp>
#include <functional>

int main()
{
  we::mgmt::requirement_t<unsigned int> p_mandatory = we::mgmt::make_mandatory(1);
  we::mgmt::requirement_t<unsigned int> p_optional  = we::mgmt::make_optional(1);

  const unsigned int wanted[] =
    {
      1, 0, 2
    };
  const unsigned int blocked[] =
    {
      9, 7, 3, 4
    };
  p_preferred.want (wanted, wanted+sizeof(wanted)/sizeof(unsigned int));
  p_mandatory.want (wanted, wanted+sizeof(wanted)/sizeof(unsigned int));

  p_preferred.cant (blocked, blocked+sizeof(blocked)/sizeof(unsigned int));
  p_mandatory.cant (blocked, blocked+sizeof(blocked)/sizeof(unsigned int));

  std::cout << "preferred: " << p_preferred << std::endl;
  std::cout << "mandatory: " << p_mandatory << std::endl;

  std::vector<unsigned int> workers;
  for (preference_t<unsigned int>::value_type rank(0); rank < 10; ++rank)
  {
    workers.push_back (rank);
  }
  std::cout << "available workers: "
            << fhg::util::show (workers.begin (), workers.end())
            << std::endl;

  for ( std::vector<unsigned int>::const_iterator w (workers.begin())
      ; w != workers.end()
      ; ++w
      )
  {
    std::cout << "pref/mand: can (" << *w << ")? "
              << p_preferred.can(*w) << "/" << p_mandatory.can(*w)
              << std::endl;
  }

  std::vector<unsigned int>::iterator bound;
  std::cout << std::endl;
  std::cout << "preferred" << std::endl;
  std::cout << "=========" << std::endl << std::endl;

  bound = ( std::stable_partition ( workers.begin()
                                  , workers.end()
                                  , p_preferred
                                  )
          );

  std::cout << "possible workers: "
            << fhg::util::show (workers.begin (), bound)
            << std::endl;
  std::cout << "impossible workers: "
            << fhg::util::show (bound, workers.end())
            << std::endl;

  std::cout << std::endl;
  std::cout << "mandatory" << std::endl;
  std::cout << "=========" << std::endl << std::endl;

  bound = ( std::stable_partition ( workers.begin()
                                  , workers.end()
                                  , p_mandatory
                                  )
          );

  std::cout << "possible workers: "
            << fhg::util::show (workers.begin (), bound)
            << std::endl;
  std::cout << "impossible workers: "
            << fhg::util::show (bound, workers.end())
            << std::endl;

  return EXIT_SUCCESS;
}
