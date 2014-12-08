// bernd.loerwald@itwm.fraunhofer.de

#include <boost/filesystem/path.hpp>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace fhg
{
  namespace rif
  {
    //! \todo ensure thread safety: uses invocation specific global state
    //!       for signal handling: let signal handler dispatch to correct
    //!       invocation instead using a map or alike
    //! \note child shall not exit with >= 240: used for execve-error handling
    //! \note child shall send end_sentinel_value before closing pipe to
    //!       indicate success or close pipe / exit before opening pipe to
    //!       indicate error. exiting after closing pipe with sentinel
    //!       sent is not an error
    //! \note returned messages do not contain end_sentinel_value
    std::pair<pid_t, std::vector<std::string>> execute_and_get_startup_messages
      ( std::string const& startup_messages_pipe_option
      , std::string const& end_sentinel_value
      , boost::filesystem::path const& command
      , std::vector<std::string> const& arguments
      , std::unordered_map<std::string, std::string> const& environment
      );
  }
}
