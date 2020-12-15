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

#pragma once

#include <we/type/bytearray.hpp>
#include <we/type/value.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <functional>
#include <map>
#include <string>

namespace stochastic_with_heureka
{
  struct workflow_result
  {
    workflow_result
      (std::multimap<std::string, pnet::type::value::value_type> const&);

    we::type::bytearray const& result() const
    {
      return _result;
    }
    bool got_heureka() const
    {
      return _got_heureka;
    }
    unsigned long number_of_rolls_done() const
    {
      return _number_of_rolls_done;
    }

  private:
    we::type::bytearray _result;
    bool _got_heureka;
    unsigned long _number_of_rolls_done;
  };

  workflow_result run
    ( int argc
    , char** argv
    , boost::optional<boost::program_options::options_description>
    , std::function< boost::filesystem::path
                       ( boost::program_options::variables_map const&
                       , boost::filesystem::path installation_dir
                       )
                   > implementation
    , std::function< we::type::bytearray
                     (boost::program_options::variables_map const&)
                   >
    );

  workflow_result run
    ( int argc
    , char** argv
    , boost::optional<boost::program_options::options_description>
    , std::string const implementation
    , std::function< we::type::bytearray
                     (boost::program_options::variables_map const&)
                   >
    );
}
