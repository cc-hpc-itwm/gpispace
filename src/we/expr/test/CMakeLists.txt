# This file is part of GPI-Space.
# Copyright (C) 2022 Fraunhofer ITWM
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

fhg_add_test (NAME we_expr_parser
  SOURCES parser.cpp
  USE_BOOST
  LIBRARIES pnet fhg-util
)

fhg_add_test (NAME we_expr_parser_performance
  SOURCES parser.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  LIBRARIES pnet fhg-util
)

fhg_add_test (NAME we_expr_parser_type_check
  SOURCES parser.type_check.cpp
  USE_BOOST
  LIBRARIES pnet fhg-util
)

fhg_add_test (NAME we_expr_type_Context
  SOURCES "type/Context.cpp"
  USE_BOOST
  LIBRARIES pnet we_expr_type_testing fhg-util Util::Generic
)
fhg_add_test (NAME we_expr_type_AssignResult
  SOURCES "type/AssignResult.cpp"
  USE_BOOST
  LIBRARIES pnet we_expr_type_testing fhg-util Util::Generic
)
