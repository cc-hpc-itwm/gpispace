# This file is part of GPI-Space.
# Copyright (C) 2020 Fraunhofer ITWM
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

fhg_add_test (NAME util_num
  SOURCES num.cpp
  USE_BOOST
  LIBRARIES fhg-util
)

fhg_add_test (NAME util_remove_prefix
  SOURCES remove_prefix.cpp
  USE_BOOST
  LIBRARIES fhg-util
)

fhg_add_test (NAME util_xml_stream
  SOURCES xml_stream.cpp
  USE_BOOST
  LIBRARIES fhg-util
)

fhg_add_test (NAME util_event
  SOURCES event.cpp
  USE_BOOST
  LIBRARIES Boost::thread Boost::system fhg-util
)

fhg_add_test (NAME util_queue
  SOURCES queue.cpp
  USE_BOOST
  LIBRARIES Boost::thread Boost::system fhg-util
)

fhg_add_test (NAME util_assert
  SOURCES assert.cpp
  USE_BOOST
  COMPILE_FLAGS "-UFHG_ASSERT_MODE -DFHG_ASSERT_MODE=1"
  LIBRARIES fhg-util
  )

fhg_add_test (NAME util_bounded_queue
  SOURCES bounded_queue.cpp
  USE_BOOST
  LIBRARIES Boost::thread Boost::system fhg-util
)
