#ifndef APPLICATION_EXEC_HPP
#define APPLICATION_EXEC_HPP 1

#include <string>
#include <iostream>

namespace process
{
  extern std::size_t execute ( std::string const & command
                             , const void * input
                             , const std::size_t & input_size
                             , void * output
                             , const std::size_t & max_output_size
                             );
}

#endif
