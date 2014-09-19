// mirko.rahn@itwm.fraunhofer.de

#ifndef SHARE_EXAMPLE_STREAM_MARK_FREE_HPP
#define SHARE_EXAMPLE_STREAM_MARK_FREE_HPP

#include <statistic.hpp>

namespace share_example_stream
{
  void mark_free ( char flag_value
                 , std::pair<void*, unsigned long> ptr_flag
                 , std::chrono::high_resolution_clock::rep produced
                 )
  {
    static fhg::util::statistic delta ("mark_free: delta");
    delta.tick (delta.now() - produced);

    char* flag (static_cast<char*> (ptr_flag.first));

    *flag = 1 - flag_value;
  }
}

#endif
