// Copyright (C) 2013-2014,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/weparse.hpp>

#include <iostream>
#include <sstream>



    namespace gspc::xml::parse::util
    {
      namespace
      {
        std::string
        format_parse_error ( std::string const& input
                           , gspc::we::expr::exception::parse::exception const& e
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

      gspc::we::expr::parse::parser generic_we_parse ( std::string const& input
                                           , std::string const& descr
                                           )
      {
        try
          {
            return gspc::we::expr::parse::parser (input);
          }
        catch (gspc::we::expr::exception::eval::divide_by_zero const& e)
          {
            throw error::weparse (descr + ": " + e.what());
          }
        catch (gspc::we::expr::exception::eval::type_error const& e)
          {
            throw error::weparse (descr + ": " + e.what());
          }
        catch (gspc::we::expr::exception::parse::exception const& e)
          {
            throw error::weparse (descr + ": " + format_parse_error (input, e));
          }
      }

      gspc::we::expr::parse::parser
      we_parse ( std::string const& input
               , std::string const& descr
               , std::string const& type
               , std::string const& name
               , std::filesystem::path const& path
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
