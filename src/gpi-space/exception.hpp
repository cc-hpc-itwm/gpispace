#pragma once

#include <stdexcept>
#include <boost/lexical_cast.hpp>

#include <gpi-space/types.hpp>

#include <string>

namespace gpi
{
  namespace error
  {
    namespace errc
    {
      enum errc_t
        {
          success = 0,

          startup_failed,

          write_dma_failed,
          read_dma_failed,

          internal_error,
        };
    }

    class code_t
    {
    public:
      explicit code_t (int val, std::string const & txt, std::string const & detail)
        : m_val (val)
        , m_txt (txt)
        , m_detail (detail)
      {}

      const std::string & name() const    { return m_txt; }
      const std::string & detail () const { return m_detail; }
      int value () const { return m_val; }
    private:
      int m_val;
      std::string m_txt;
      std::string m_detail;
    };

#define MK_CODE_T(name)                         \
    struct name : public code_t                 \
    {                                           \
      name (std::string const & detail="")      \
        : code_t (errc::name, #name, detail)    \
      {}                                        \
    }

    MK_CODE_T(startup_failed);

    MK_CODE_T(write_dma_failed);
    MK_CODE_T(read_dma_failed);

    MK_CODE_T(internal_error);

#undef MK_CODE_T
  }
}

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

      virtual ~gpi_error () = default;

      virtual const char * what () const noexcept override
      {
        return message.c_str();
      }

      const int value;
      const std::string user_message;
      const std::string message;
    };
  }
}
