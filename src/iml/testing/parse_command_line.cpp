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

#include <iml/testing/parse_command_line.hpp>

#include <boost/program_options/parsers.hpp>

namespace iml_test
{
  boost::program_options::variables_map parse_command_line
    ( int argc
    , char** argv
    , boost::program_options::options_description const& options
    )
  {
    boost::program_options::variables_map vm;
    boost::program_options::command_line_parser impl (argc, argv);
    impl.options (options);
    boost::program_options::store (impl.run(), vm);
    return vm;
  }
}
