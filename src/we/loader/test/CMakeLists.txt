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

fhg_add_test (NAME we_loader_loader
  SOURCES loader.cpp
  USE_BOOST
  LIBRARIES pnet
            order_stack
            ${CMAKE_DL_LIBS}
  DEPENDS question
          answer
          order_a
          order_b
)

fhg_add_test (NAME we_loader_module
  SOURCES module.cpp
  USE_BOOST
  LIBRARIES pnet
            ${CMAKE_DL_LIBS}
  DEPENDS initialize_throws
          empty_not_linked_with_pnet
          empty
)

extended_add_library (NAME answer TYPE SHARED SOURCES "question_answer/answer.cpp" LIBRARIES pnet)
extended_add_library (NAME question TYPE SHARED SOURCES "question_answer/question.cpp" LIBRARIES answer)
extended_add_library (NAME order_stack TYPE SHARED SOURCES "order/stack.cpp")
extended_add_library (NAME order_a TYPE SHARED SOURCES "order/a.cpp" LIBRARIES order_stack pnet)
extended_add_library (NAME order_b TYPE SHARED SOURCES "order/b.cpp" LIBRARIES order_stack pnet)
extended_add_library (NAME initialize_throws TYPE SHARED SOURCES "module/initialize_throws.cpp" LIBRARIES pnet)
extended_add_library (NAME empty_not_linked_with_pnet
  TYPE SHARED
  SOURCES "module/empty_not_linked_with_pnet.cpp"
  LIBRARIES Boost::base
)
extended_add_library (NAME empty TYPE SHARED SOURCES "module/empty" LIBRARIES pnet)
extended_add_library (NAME empty_nodelete
  TYPE SHARED
  SOURCES "module/empty.cpp"
  LIBRARIES pnet
)
target_link_options (empty_nodelete PUBLIC LINKER:-z,nodelete)
