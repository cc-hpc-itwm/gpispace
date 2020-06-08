#pragma once

#include <boost/program_options.hpp>

namespace test
{
  void set_virtual_memory_socket_name_for_localhost
    (boost::program_options::variables_map&);
}
