// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/print_exception.hpp>

#include <utility>

namespace fhg
{
  namespace util
  {
    namespace
    {
      void prefix ( std::ostream& os
                  , ::boost::optional<std::string> static_separator
                  , ::boost::optional<std::size_t> indentation
                  , bool is_first
                  )
      {
        if (!is_first)
        {
          os << static_separator.get_value_or ("\n");
        }
        os << std::string (indentation.get_value_or (0), ' ');
      }
      ::boost::optional<std::size_t> maybe_inc
        (::boost::optional<std::size_t> x)
      {
        if (x)
        {
          return ++(*x);
        }
        return x;
      }

      void print_unknown ( std::ostream& os
                         , ::boost::optional<std::string> static_separator
                         , ::boost::optional<std::size_t> indentation
                         , bool is_first
                         )
      {
        prefix (os, static_separator, indentation, is_first);
        os << "unknown exception type";
      }

      void print_exception ( std::ostream& os
                           , std::exception const& e
                           , ::boost::optional<std::string> static_separator
                           , ::boost::optional<std::size_t> indentation
                           , bool is_first
                           )
      {
        prefix (os, static_separator, indentation, is_first);
        os << e.what();

        try
        {
          std::rethrow_if_nested (e);
        }
        catch (std::exception const& catched)
        {
          print_exception
            (os, catched, static_separator, maybe_inc (indentation), false);
        }
        catch (...)
        {
          print_unknown
            (os, static_separator, maybe_inc (indentation), false);
        }
      }

      void print_exception ( std::ostream& os
                           , std::exception_ptr const& e
                           , ::boost::optional<std::string> static_separator
                           , ::boost::optional<std::size_t> indentation
                           , bool is_first
                           )
      {
        try
        {
          std::rethrow_exception (e);
        }
        catch (std::exception const& catched)
        {
          print_exception
            (os, catched, static_separator, indentation, is_first);
        }
        catch (...)
        {
          print_unknown (os, static_separator, indentation, is_first);
        }
      }
    }

    exception_printer::exception_printer ( std::exception_ptr exception
                                         , std::size_t indentation_base
                                         )
      : exception_printer (exception, indentation_base, ::boost::none)
    {}

    exception_printer::exception_printer ( std::exception_ptr exception
                                         , std::string separator
                                         )
      : exception_printer (exception, ::boost::none, std::move (separator))
    {}

    exception_printer::exception_printer ( std::exception_ptr exception
                                         , ::boost::optional<std::size_t> indent
                                         , ::boost::optional<std::string> separator
                                         )
      : _exception (exception)
      , _separate_with_line_break_and_indent_base (std::move (indent))
      , _separate_static (std::move (separator))
    {}

    current_exception_printer::current_exception_printer (std::size_t indentation_base)
      : exception_printer (std::current_exception(), indentation_base)
    {}

    current_exception_printer::current_exception_printer (std::string separator)
      : exception_printer (std::current_exception(), std::move (separator))
    {}

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
