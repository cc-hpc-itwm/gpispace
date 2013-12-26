#ifndef GSPC_CTL_LOG_CMD_HPP
#define GSPC_CTL_LOG_CMD_HPP

#include <string>
#include <vector>

namespace gspc
{
  namespace ctl
  {
    namespace cmd
    {
      int log_cmd (std::vector<std::string> const& argv);
    }
  }
}

#endif
