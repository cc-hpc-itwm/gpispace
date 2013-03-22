// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_WEPARSE_HPP
#define _XML_PARSE_UTIL_WEPARSE_HPP

#include <we/expr/parse/parser.hpp>

#include <xml/parse/error.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      std::string
      format_parse_error ( const std::string&
                         , const expr::exception::parse::exception&
                         );

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

#endif
