// mirko.rahn@itwm.fraunhofer.de

#ifndef SHARE_EXAMPLE_STREAM_PROCESS_HPP
#define SHARE_EXAMPLE_STREAM_PROCESS_HPP

#include <statistic.hpp>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

namespace share_example_stream
{
  std::chrono::high_resolution_clock::rep
    process ( std::string const& log_file
            , std::pair<void const*, unsigned long> ptr_data
            )
  {
    static fhg::util::statistic delta ("process: delta");
    static fhg::util::statistic duration ("process: duration");

    std::chrono::high_resolution_clock::rep const start (delta.now());

    char const* const data (static_cast<char const* const> (ptr_data.first));

    std::istringstream iss (std::string (data, data + ptr_data.second));

    unsigned long id;
    iss >> id;

    std::chrono::high_resolution_clock::rep produced;
    iss >> produced;

    delta.tick (start - produced);

    std::ofstream log (log_file.c_str(), std::ios_base::app);

    log << id << " " << produced << std::endl;

    duration.tick (duration.now() - start);

    return produced;
  }
}

#endif
