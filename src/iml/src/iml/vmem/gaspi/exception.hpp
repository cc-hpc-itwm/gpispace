// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/vmem/gaspi/types.hpp>

#include <stdexcept>
#include <string>

namespace gpi
{
  namespace error
  {
    namespace errc
    {
      enum errc_t
        { startup_failed,
          internal_error
        };
    }

    class code_t
    {
    public:
      explicit code_t (int val, std::string const& txt)
        : m_val (val)
        , m_txt (txt)
      {}

      std::string const& name() const    { return m_txt; }
      int value () const { return m_val; }
    private:
      int m_val;
      std::string m_txt;
    };

#define MK_CODE_T(name)                         \
    struct name : public code_t                 \
    {                                           \
      name ()                                   \
        : code_t (errc::name, #name)            \
      {}                                        \
    }

    MK_CODE_T(startup_failed);
    MK_CODE_T(internal_error);

#undef MK_CODE_T
  }
}

namespace gpi
{
  namespace exception
  {
    struct gaspi_error : public std::runtime_error
    {
      explicit
      gaspi_error (gpi::error::code_t const& ec, std::string const& m)
        : std::runtime_error (ec.name ())
        , value (ec.value())
        , user_message (m)
        , message ("gaspi::error[" + std::to_string (value) + "]: " + ec.name() + ": " + user_message)
      {}

      ~gaspi_error () override = default;
      gaspi_error (gaspi_error const&) = delete;
      gaspi_error (gaspi_error&&) = default;
      gaspi_error& operator= (gaspi_error const&) = delete;
      gaspi_error& operator= (gaspi_error&&) = delete;

      const char * what () const noexcept override
      {
        return message.c_str();
      }

      const int value;
      const std::string user_message;
      const std::string message;
    };
  }
}
