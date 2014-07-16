// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_DRTS_HPP
#define DRTS_DRTS_HPP

#include <boost/filesystem.hpp>

#include <string>

namespace gspc
{
  struct scoped_runtime_system
  {
    scoped_runtime_system ( std::string const& command_boot
                          , boost::filesystem::path const& gspc_home
                          , boost::filesystem::path const& state_directory
                          );

    ~scoped_runtime_system();

  private:
    boost::filesystem::path const& _gspc_home;
    boost::filesystem::path const& _state_directory;
  };
}

#endif
