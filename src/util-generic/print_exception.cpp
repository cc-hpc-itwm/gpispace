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

#include <util-generic/print_exception.hpp>

#include <typeinfo>

namespace fhg
{
  namespace util
  {
    namespace
    {
      void prefix ( std::ostream& os
                  , boost::optional<std::string> static_separator
                  , boost::optional<std::size_t> indentation
                  , bool is_first
                  )
      {
        if (!is_first)
        {
          os << static_separator.get_value_or ("\n");
        }
        os << std::string (indentation.get_value_or (0), ' ');
      }
      boost::optional<std::size_t> maybe_inc
        (boost::optional<std::size_t> x)
      {
        if (x)
        {
          return ++(*x);
        }
        return x;
      }

      void print_unknown ( std::ostream& os
                         , boost::optional<std::string> static_separator
                         , boost::optional<std::size_t> indentation
                         , bool is_first
                         )
      {
        prefix (os, static_separator, indentation, is_first);
        os << "unknown exception type";
      }

      void print_exception ( std::ostream& os
                           , std::exception const& e
                           , boost::optional<std::string> static_separator
                           , boost::optional<std::size_t> indentation
                           , bool is_first
                           )
      {
        prefix (os, static_separator, indentation, is_first);
        os << e.what();

        try
        {
          std::rethrow_if_nested (e);
        }
        catch (std::exception const& e)
        {
          print_exception
            (os, e, static_separator, maybe_inc (indentation), false);
        }
        catch (...)
        {
          print_unknown
            (os, static_separator, maybe_inc (indentation), false);
        }
      }

      void print_exception ( std::ostream& os
                           , std::exception_ptr const& e
                           , boost::optional<std::string> static_separator
                           , boost::optional<std::size_t> indentation
                           , bool is_first
                           )
      {
        try
        {
          std::rethrow_exception (e);
        }
        catch (std::exception const& e)
        {
          print_exception (os, e, static_separator, indentation, is_first);
        }
        catch (...)
        {
          print_unknown (os, static_separator, indentation, is_first);
        }
      }
    }

    std::ostream& exception_printer::operator() (std::ostream& os) const
    {
      print_exception ( os
                      , _exception
                      , _separate_static
                      , _separate_with_line_break_and_indent_base
                      , true
                      );

      return os;
    }
  }
}
