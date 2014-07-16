// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_DRTS_HPP
#define DRTS_DRTS_HPP

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <string>

namespace gspc
{
  namespace options
  {
    boost::program_options::options_description logging();
  }

  struct scoped_runtime_system
  {
    scoped_runtime_system ( std::string const& command_boot
                          , boost::program_options::variables_map const& vm
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
