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
    boost::program_options::options_description drts();
  }

  struct scoped_runtime_system
  {
    scoped_runtime_system ( std::string const& command_boot
                          , boost::program_options::variables_map const& vm
                          );

    ~scoped_runtime_system();

    boost::filesystem::path const& gspc_home() const
    {
      return _gspc_home;
    }
    boost::filesystem::path const& state_directory() const
    {
      return _state_directory;
    }
    boost::filesystem::path const& nodefile() const
    {
      return _nodefile;
    }

  private:
    boost::filesystem::path const _gspc_home;
    boost::filesystem::path const _state_directory;
    boost::filesystem::path const _nodefile;
  };
}

#endif
