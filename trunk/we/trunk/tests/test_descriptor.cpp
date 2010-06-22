#include <iostream>
#include <we/we.hpp>
#include <we/mgmt/bits/descriptor.hpp>
#include <boost/bind.hpp>

typedef we::mgmt::detail::descriptor< unsigned int
                                    , we::activity_t
                                    > descriptor_t;


static void print_child (unsigned int c)
{
  std::cout << "Child-" << c << std::endl;
}

static void del_child (descriptor_t & d, unsigned int c)
{
  std::cout << "removing Child-" << c << std::endl;
  d.del_child (c);
  std::cout << d << std::endl;
}

int main()
{
  descriptor_t d0 (0, we::activity_t ());

  d0.add_child (1);
  d0.add_child (2);

  d0.apply_to_children (&print_child);

  std::cout << d0 << std::endl;

  d0.apply_to_children (boost::bind (&del_child, d0, _1));

  descriptor_t d1 (1, we::activity_t (), 0);
  std::cout << d1 << std::endl;

  return EXIT_SUCCESS;
}
