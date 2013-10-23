#ifndef GSPC_CTL_EVAL_HPP
#define GSPC_CTL_EVAL_HPP

#include <boost/filesystem.hpp>

#include <vector>
#include <string>

namespace gspc
{
  namespace ctl
  {
    int eval ( std::vector<std::string> const & argv
             , std::string & out
             , std::string & err
             , const std::string & inp = ""
             );
  }
}

#endif
