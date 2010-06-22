#include <iostream>
#include <we/mgmt/type/preference.hpp>

using namespace we::mgmt::pref;

int main()
{
  preference_t p;

  p.want (0).want (1).want (2);
  p.cant (3).cant (4);
  p += 5;
  p -= 6;

  std::cout << p << std::endl;

  for (preference_t::value_type rank(0); rank < 10; ++rank)
  {
    std::cout << "can (" << rank << ")? " << p.can(rank) << std::endl;
  }

  return EXIT_SUCCESS;
}
