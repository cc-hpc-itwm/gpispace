#ifndef GPI_SPACE_EXCEPTION_HPP
#define GPI_SPACE_EXCEPTION_HPP 1

#include <stdexcept>
#include <boost/lexical_cast.hpp>

#include <gpi-space/types.hpp>
#include <gpi-space/error.hpp>

namespace gpi
{
  namespace exception
  {
    struct gpi_error : public std::runtime_error
    {
      explicit
      gpi_error (gpi::error::code_t const & ec)
        : std::runtime_error (ec.name())
        , value (ec.value())
        , user_message (ec.detail())
        , message ("gpi::error[" + boost::lexical_cast<std::string>(value) + "]: " + ec.name())
      {}

      explicit
      gpi_error (gpi::error::code_t const & ec, std::string const & m)
        : std::runtime_error (ec.name ())
        , value (ec.value())
        , user_message (m)
        , message ("gpi::error[" + boost::lexical_cast<std::string>(value) + "]: " + ec.name() + ": " + user_message)
      {}

      virtual ~gpi_error () throw () {}

      virtual const char * what () const throw ()
      {
        return message.c_str();
      }

      int value;
      const std::string user_message;
      const std::string message;
    };
  }
}

#endif
