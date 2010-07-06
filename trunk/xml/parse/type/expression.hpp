// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_EXPRESSION_HPP
#define _XML_PARSE_TYPE_EXPRESSION_HPP

#include <iostream>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct expression
      {
      public:
        std::string term;
        boost::filesystem::path path;

        expression ()
          : term ()
          , path ()
        {}

        expression ( const std::string & _term
                   , const boost::filesystem::path & _path
                   )
          : term (_term)
          , path (_path)
        {}
      };

      std::ostream & operator << (std::ostream & s, const expression & e)
      {
        return s << "expression ("
                 << "term = " << e.term 
                 << ", path = " << e.path
                 << ")"
          ;
      }
    }
  }
}

#endif
