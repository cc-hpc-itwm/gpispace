// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_DRTS_WORKER_START_DRTS_KERNEL_HPP
#define FHG_DRTS_WORKER_START_DRTS_KERNEL_HPP

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <string>
#include <utility>
#include <vector>

namespace fhg
{
  namespace drts
  {
    namespace worker
    {
      void start ( bool verbose
                 , std::string gui
                 , boost::optional<std::string> log_url
                 , std::string master
                 , unsigned long identity
                 , boost::optional<std::pair<boost::filesystem::path, unsigned long>> gpi
                 , std::vector<std::string> capabilities
                 , std::vector<boost::filesystem::path> lib_path
                 , boost::filesystem::path state_dir
                 , std::string name_prefix
                 , boost::optional<unsigned long> numa_socket
                 , boost::filesystem::path installation_dir
                 );
    }
  }
}

#endif
