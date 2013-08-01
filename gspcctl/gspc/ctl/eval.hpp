#ifndef GSPC_CTL_EVAL_HPP
#define GSPC_CTL_EVAL_HPP

#include <boost/filesystem.hpp>

#include <string>

namespace gspc
{
  namespace ctl
  {
    int eval ( boost::filesystem::path const & cmd
             , char *argv[]
             , size_t argc
             , std::string & out
             , std::string & err
             , const std::string & inp = ""
             );
  }
}

#endif
