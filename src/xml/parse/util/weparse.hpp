#pragma once

#include <we/expr/parse/parser.hpp>

#include <xml/parse/error.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      expr::parse::parser generic_we_parse ( const std::string& input
                                           , const std::string& descr
                                           );
      expr::parse::parser we_parse ( const std::string& input
                                   , const std::string& descr
                                   , const std::string& type
                                   , const std::string& name
                                   , const boost::filesystem::path&
                                   );
   }
  }
}
