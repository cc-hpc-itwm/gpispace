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

#include <boost/serialization/utility.hpp>

namespace fhg
{
  namespace logging
  {
    template<typename Archive>
      void serialize (Archive& ar, socket_endpoint& ep, unsigned int const)
    {
      ar & ep.host;

      std::string path;
      if (typename Archive::is_saving{})
      {
        path = ep.socket.path();
      }
      ar & path;
      if (typename Archive::is_loading{})
      {
        ep.socket = fhg::logging::socket_endpoint::Socket (path);
      }
    }
  }
}
