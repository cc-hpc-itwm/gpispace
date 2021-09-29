# This file is part of GPI-Space.
# Copyright (C) 2021 Fraunhofer ITWM
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

fhg_add_test (NAME fhgcom_p2p
  SOURCES p2p.cpp
  USE_BOOST
  LIBRARIES fhgcom
            Util::Generic-Headers
)

fhg_add_test (NAME fhgcom_peer
  SOURCES peer.cpp
  USE_BOOST
  LIBRARIES fhgcom
            fhg-util
            test-util-certificates
            Boost::filesystem
            Boost::serialization
            Boost::date_time
            Boost::thread
            OpenSSL::SSL
            OpenSSL::Crypto
)
