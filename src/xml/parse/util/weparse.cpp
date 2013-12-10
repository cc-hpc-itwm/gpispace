// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/weparse.hpp>

#include <sstream>
#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      std::string
      format_parse_error ( const std::string& input
                         , const expr::exception::parse::exception& e
                         )
      {
        std::ostringstream s;

        s << std::endl << input << std::endl;
        for (std::size_t k (0); k < e.eaten; ++k)
          {
            s << " ";
          }
        s << "^" << std::endl;
        s << e.what() << std::endl;

        return s.str();
      }

      expr::parse::parser generic_we_parse ( const std::string& input
                                           , const std::string& descr
                                           )
      {
        try
          {
            return expr::parse::parser (input);
          }
        catch (const expr::exception::eval::divide_by_zero & e)
          {
            throw error::weparse (descr + ": " + e.what());
          }
        catch (const expr::exception::eval::type_error & e)
          {
            throw error::weparse (descr + ": " + e.what());
          }
        catch (const expr::exception::parse::exception & e)
          {
            throw error::weparse (descr + ": " + format_parse_error (input, e));
          }
      }

      expr::parse::parser
      we_parse ( const std::string& input
               , const std::string& descr
               , const std::string& type
               , const std::string& name
               , const boost::filesystem::path& path
               )
      {
        std::ostringstream s;

        s << "when parsing " << descr
          << " of " << type << " " << name
          << " in " << path
          ;

        return generic_we_parse (input, s.str());
      }
   }
  }
}
