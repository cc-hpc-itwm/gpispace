// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <xml/parse/util/weparse.hpp>

#include <sstream>
#include <iostream>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      namespace
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
