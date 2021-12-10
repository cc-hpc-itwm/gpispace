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

#include <iml/gaspi/NetdevID.hpp>

#include <util-generic/boost/program_options/validators.hpp>

#include <boost/any.hpp>
#include <boost/program_options/errors.hpp>

namespace iml
{
  namespace gaspi
  {
    NetdevID::NetdevID (std::string const& option)
      : value (option == "0" ? 0 : option == "1" ? 1 : -1)
    {
      if (option != "auto" && value != 0 && value != 1)
      {
        throw ::boost::program_options::invalid_option_value
          ("Expected 'auto' or '0' or '1', got '" + option + "'");
      }
    }

    std::string NetdevID::to_string() const
    {
      return value == -1 ? "auto" : std::to_string (value);
    }

    std::ostream& operator<< (std::ostream& os, NetdevID const& id)
    {
      return os << id.to_string();
    }

    void validate
      (::boost::any& v, std::vector<std::string> const& values, NetdevID*, int)
    {
      fhg::util::boost::program_options::validate<NetdevID> (v, values);
    }
  }
}
