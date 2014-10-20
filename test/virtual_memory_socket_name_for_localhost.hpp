// mirko.rahn@itwm.fraunhofer.de

#ifndef TEST_VIRTUAL_MEMORY_SOCKET_NAME_FOR_LOCALHOST_HPP
#define TEST_VIRTUAL_MEMORY_SOCKET_NAME_FOR_LOCALHOST_HPP

#include <boost/program_options.hpp>

namespace test
{
  void set_virtual_memory_socket_name_for_localhost
    (boost::program_options::variables_map&);
}

#endif
