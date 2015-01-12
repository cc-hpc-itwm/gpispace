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
    //! \note child shall not exit with >= 240: used for execve-error handling
    //! \note child shall send end_sentinel_value before closing pipe to
    //!       indicate success or close pipe / exit before opening pipe to
    //!       indicate error. exiting after closing pipe with sentinel
    //!       sent is not an error
    //! \note returned messages do not contain end_sentinel_value
    std::pair<pid_t, std::vector<std::string>> execute_and_get_startup_messages
      ( std::string startup_messages_pipe_option
      , std::string end_sentinel_value
      , boost::filesystem::path command
      , std::vector<std::string> arguments
      , std::unordered_map<std::string, std::string> environment
      );
  }
}
