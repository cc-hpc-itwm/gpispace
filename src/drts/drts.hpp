// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_DRTS_HPP
#define DRTS_DRTS_HPP

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <string>

namespace gspc
{
  namespace options
  {
    boost::program_options::options_description logging();
    boost::program_options::options_description drts();
    boost::program_options::options_description virtual_memory();
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
    boost::optional<unsigned long> const& virtual_memory_per_node() const
    {
      return _virtual_memory_per_node;
    }
    boost::optional<boost::filesystem::path> const& virtual_memory_socket() const
    {
      return _virtual_memory_socket;
    }

  private:
    boost::filesystem::path const _gspc_home;
    boost::filesystem::path const _state_directory;
    boost::filesystem::path const _nodefile;
    boost::optional<unsigned long> _virtual_memory_per_node;
    boost::optional<boost::filesystem::path> _virtual_memory_socket;
  };
}

#endif
