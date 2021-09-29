// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <stdexcept>
#include <boost/lexical_cast.hpp>

#include <iml/vmem/gaspi/types.hpp>

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
        , message ("gaspi::error[" + boost::lexical_cast<std::string>(value) + "]: " + ec.name() + ": " + user_message)
      {}

      virtual ~gaspi_error () override = default;

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
