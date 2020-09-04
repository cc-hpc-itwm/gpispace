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

#pragma once

#include <util-generic/ostream/modifier.hpp>

#include <boost/optional.hpp>

#include <exception>
#include <ostream>
#include <string>
#include <utility>

namespace fhg
{
  namespace util
  {
    class exception_printer : public ostream::modifier
    {
    public:
      exception_printer ( std::exception_ptr exception
                        , std::size_t indentation_base = 0
                        )
        : exception_printer (exception, indentation_base, boost::none)
      {}

      exception_printer ( std::exception_ptr exception
                        , std::string separator
                        )
        : exception_printer (exception, boost::none, std::move (separator))
      {}

      virtual ~exception_printer() override = default;
      virtual std::ostream& operator() (std::ostream&) const override;

    private:
      exception_printer ( std::exception_ptr exception
                        , boost::optional<std::size_t> indent
                        , boost::optional<std::string> separator
                        )
        : _exception (exception)
        , _separate_with_line_break_and_indent_base (std::move (indent))
        , _separate_static (std::move (separator))
      {}

      std::exception_ptr _exception;
      //! \todo either<>
      boost::optional<std::size_t>
        _separate_with_line_break_and_indent_base;
      boost::optional<std::string> _separate_static;
    };

    struct current_exception_printer : public exception_printer
    {
      current_exception_printer (std::size_t indentation_base = 0)
        : exception_printer (std::current_exception(), indentation_base)
      {}

      current_exception_printer (std::string separator)
        : exception_printer
            (std::current_exception(), std::move (separator))
      {}
    };
  }
}
