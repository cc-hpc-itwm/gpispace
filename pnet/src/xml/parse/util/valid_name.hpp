// mirko.rahn@itwm.fraunhofer.de

#ifndef XML_PARSE_VALID_NAME_HPP
#define XML_PARSE_VALID_NAME_HPP

#include <fhg/util/parse/position.hpp>

#include <boost/filesystem/path.hpp>

#include <string>

namespace xml
{
  namespace parse
  {
    //! \note [:space:]*([:alpha:_][:alpha::num:_]*)[:space:]*
    std::string parse_name (fhg::util::parse::position& pos);

    //! \note parse_name (name) == name
    std::string validate_name ( const std::string & name
                              , const std::string & type
                              , const boost::filesystem::path & path
                              );
  }
}

#endif
