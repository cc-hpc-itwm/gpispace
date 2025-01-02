// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/gaspi/NetdevID.hpp>

#include <util-generic/boost/program_options/validators.hpp>

#include <boost/any.hpp>
#include <boost/program_options/errors.hpp>

#include <stdexcept>

namespace iml
{
  namespace gaspi
  {
    NetdevID::NetdevID (std::string const& option)
      : value (-1)
    {
      // rewrap exception for backwards-compatibility
      try
      {
        value = NetdevID::from_string (option);
      }
      catch (...)
      {
        throw ::boost::program_options::invalid_option_value
            ("Expected 'auto' or '<device-id> (e.g. '0', '1', ...), got '" + option + "'");
      }
    }

    int NetdevID::from_string(std::string const& option)
    {
      int value = -1;

      if (option != "auto")
      {
        value = std::stoi (option);
        if (value < -1)
        {
          throw std::invalid_argument ("Invalid Device ID");
        }
      }

      return value;
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
