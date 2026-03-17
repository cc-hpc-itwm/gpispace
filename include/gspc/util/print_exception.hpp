#pragma once

#include <gspc/util/ostream/modifier.hpp>

#include <optional>

#include <cstddef>
#include <exception>
#include <ostream>
#include <string>


  namespace gspc::util
  {
    //! An ostream modifier to print an exception pointer, including
    //! nested exceptions.
    class exception_printer : public ostream::modifier
    {
    public:
      exception_printer ( std::exception_ptr exception
                        , std::size_t indentation_base = 0
                        );

      exception_printer ( std::exception_ptr exception
                        , std::string separator
                        );

      ~exception_printer() override = default;
      std::ostream& operator() (std::ostream&) const override;

      exception_printer (exception_printer const&) = delete;
      exception_printer& operator= (exception_printer const&) = delete;
      exception_printer (exception_printer&&) = delete;
      exception_printer& operator= (exception_printer&&) = delete;

    private:
      exception_printer ( std::exception_ptr exception
                        , std::optional<std::size_t> indent
                        , std::optional<std::string> separator
                        );

      std::exception_ptr _exception;
      //! \todo either<>
      std::optional<std::size_t> _separate_with_line_break_and_indent_base;
      std::optional<std::string> _separate_static;
    };

    //! An ostream modifier to print the currently in-flight
    //! exception, including nested exceptions.
    struct current_exception_printer : public exception_printer
    {
      current_exception_printer (std::size_t indentation_base = 0);
      current_exception_printer (std::string separator);
    };
  }
