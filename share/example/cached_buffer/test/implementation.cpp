#include <cached_buffer/interface.hpp>

#include <boost/format.hpp>

#include <stdexcept>
#include <iostream>
#include <vector>


void read_and_validate_neighbor_data
  ( std::vector<long> const& neighbor_indices
  , std::vector<long*> const& data
  , long id
  , long count
  )
{
  for (unsigned long neighbor(0); neighbor < neighbor_indices.size(); ++neighbor)
  {
    for (long i (0); i < count; ++i)
    {
        if (data.at(neighbor)[i] != (i + neighbor_indices.at(neighbor) * count))
        {
          throw std::runtime_error
              (( boost::format ("Incorrect buffer contents for id='%1%' at neighbor='%2%', element %3%: %4% != %5%")
               % id
               % neighbor_indices.at(neighbor)
               % i
               % data.at(neighbor)[i]
               % (i + neighbor_indices.at(neighbor) * count)
               ).str()
              );
        }
    }
  }
}
