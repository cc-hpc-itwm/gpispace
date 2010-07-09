// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_UTIL_WEPARSE_HPP
#define _XML_PARSE_UTIL_WEPARSE_HPP

#include <we/expr/parse/parser.hpp>
#include <we/util/read.hpp>

#include <parse/error.hpp>

#include <string>
#include <sstream>
#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      typedef expr::parse::parser<std::string> we_parser_t;

      std::string
      format_parse_error ( const std::string & input
                         , const expr::exception::parse::exception & e
                         )
      {
        std::ostringstream s;

        s << std::endl << input << std::endl;
        for (unsigned int k (0); k < e.eaten; ++k)
          {
            s << " ";
          }
        s << "^" << std::endl;
        s << e.what() << std::endl;

        return s.str();
      }

      we_parser_t we_parse ( const std::string & input
                           , const std::string & descr
                           , const std::string & type
                           , const std::string & name
                           , const boost::filesystem::path & path
                           )
      {
        try
          {
            return we_parser_t (input);
          }
        catch (const expr::exception::parse::exception & e)
          {
            std::ostringstream s;

            s << "when parsing " << descr 
              << " of " << type << " " << name
              << " in " << path
              << ":" << format_parse_error (input, e)
              ;

            throw error::weparse (s.str());
          }
      }
    }
  }
}

#endif
