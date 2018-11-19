#pragma once

#include <vector>


extern "C"
{
void read_and_validate_neighbor_data
  ( std::vector<long> const&
  , std::vector<long*> const&
  , long
  , long
  );
}
