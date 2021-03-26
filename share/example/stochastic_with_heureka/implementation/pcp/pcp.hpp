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

#pragma once

#include <we/type/bytearray.hpp>

#include <boost/optional.hpp>

#include <list>
#include <string>
#include <utility>

namespace pcp
{
  class configuration
  {
  public:
    configuration ( boost::optional<std::string> top
                  , boost::optional<std::string> bot
                  )
      : _top (top)
      , _bot (bot)
    {}
    boost::optional<configuration> apply
      (std::pair<std::string, std::string> const& rule) const;
    bool is_solution() const
    {
      return !_top && !_bot;
    }
    void clear()
    {
      _top = _bot = boost::none;
    }

  private:
    boost::optional<std::string> _top;
    boost::optional<std::string> _bot;
  };

  class pcp
  {
  public:
    pcp (std::list<std::pair<std::string, std::string>> rules)
      : _rules (rules)
    {}
    pcp (we::type::bytearray const&);
    we::type::bytearray bytearray() const;

    std::list<std::pair<std::string, std::string>> rules() const
    {
      return _rules;
    }

  private:
    std::list<std::pair<std::string, std::string>> _rules;
  };
}
