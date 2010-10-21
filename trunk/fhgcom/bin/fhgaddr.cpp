#include <iostream>

#include <fhgcom/header.hpp>

int main (int ac, char *av[])
{
  if (ac < 2)
  {
    std::cerr << "usage: " << av[0] << " peer-name" << std::endl;
    std::cerr << "   computes the unique address of the given peer" << std::endl;
    return 1;
  }

  for (int i = 1 ; i < ac ; ++i)
  {
    std::cout << fhg::com::p2p::address_t (av[i]) << std::endl;
  }
  return 0;
}
