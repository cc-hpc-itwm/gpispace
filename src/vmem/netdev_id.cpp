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

#include <vmem/netdev_id.hpp>

#include <fhg/util/boost/program_options/validators.hpp>

#include <boost/program_options/errors.hpp>

namespace fhg
{
  namespace vmem
  {
    netdev_id::netdev_id()
      : netdev_id ("auto")
    {}

    netdev_id::netdev_id (std::string const& option)
      : value (option == "0" ? 0 : option == "1" ? 1 : -1)
    {
      if (option != "auto" && value != 0 && value != 1)
      {
        throw boost::program_options::invalid_option_value
          ("Expected 'auto' or '0' or '1', got '" + option + "'");
      }
    }

    std::string to_string (netdev_id const& id)
    {
      return id.value == -1 ? "auto" : std::to_string (id.value);
    }

    std::ostream& operator<< (std::ostream& os, netdev_id const& id)
    {
      return os << to_string (id);
    }

    void validate
      (boost::any& v, std::vector<std::string> const& values, netdev_id*, int)
    {
      fhg::util::boost::program_options::validate<netdev_id> (v, values);
    }
  }
}
