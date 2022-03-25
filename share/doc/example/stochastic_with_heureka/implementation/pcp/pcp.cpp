// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <interface.hpp>

#include <implementation/pcp/pcp.hpp>

#include <cstddef>
#include <random>
#include <sstream>
#include <vector>

namespace pcp
{
  ::boost::optional<configuration>
    configuration::apply (std::pair<std::string, std::string> const& rule) const
  {
    std::string const top (_top.get_value_or ("") + rule.first);
    std::string const bot (_bot.get_value_or ("") + rule.second);

    std::string::const_iterator top_pos (top.cbegin());
    std::string::const_iterator bot_pos (bot.cbegin());

    while (top_pos != top.end() && bot_pos != bot.end() && *top_pos == *bot_pos)
    {
      ++top_pos;
      ++bot_pos;
    }

    return ::boost::make_optional
      ( top_pos == top.end() || bot_pos == bot.end()
      , configuration
        { ::boost::make_optional
          (top_pos != top.end(), std::string (top_pos, top.end()))
        , ::boost::make_optional
          (bot_pos != bot.end(), std::string (bot_pos, bot.end()))
        }
      );
  }

  we::type::bytearray pcp::bytearray() const
  {
    std::ostringstream oss;

    oss << std::to_string (_rules.size()) << ' ';

    for (std::pair<std::string, std::string> rule : _rules)
    {
      oss << std::to_string (rule.first.size()) << ' ' << rule.first
          << std::to_string (rule.second.size()) << ' ' << rule.second
        ;
    }

    return oss.str();
  }

  pcp::pcp (we::type::bytearray const& ba)
  {
    std::string s (ba.to_string());
    std::size_t pos;
    std::size_t num_rules (std::stoul (s, &pos));

    s = s.substr (pos + 1);

    while (num_rules --> 0)
    {
      std::size_t const lt (std::stoul (s, &pos));
      std::string const top (s.substr (pos + 1, lt));
      s = s.substr (pos + 1 + lt);
      std::size_t const lb (std::stoul (s, &pos));
      std::string const bot (s.substr (pos + 1, lb));
      s = s.substr (pos + 1 + lb);
      _rules.push_back ({top, bot});
    }
  }
}

extern "C"
  std::pair<we::type::bytearray, bool> stochastic_with_heureka_roll_and_heureka
    (unsigned long n, unsigned long seed, we::type::bytearray pcp_bytearray)
{
  pcp::pcp const pcp {pcp_bytearray};

  std::mt19937 generator (seed);

  std::string path;
  pcp::configuration configuration (::boost::none, ::boost::none);

  for (unsigned long i (0); i < n; ++i)
  {
    std::vector<std::pair<char, pcp::configuration>> successors;

    char rule_id ('a');

    for (std::pair<std::string, std::string> rule : pcp.rules())
    {
      ::boost::optional<pcp::configuration> successor
        (configuration.apply (rule));

      if (successor)
      {
        successors.emplace_back (rule_id, *successor);
      }

      ++rule_id;
    }

    if (successors.empty())
    {
      path.clear();
      configuration.clear();
    }
    else
    {
      std::uniform_int_distribution<std::size_t> random_number
        (0, successors.size() - 1);

      std::size_t const successor_id (random_number (generator));

      path.push_back (successors[successor_id].first);
      configuration = successors[successor_id].second;

      if (configuration.is_solution())
      {
        return {we::type::bytearray {path}, true};
      }
    }
  }

  return {we::type::bytearray {std::string()}, false};
};

extern "C"
  we::type::bytearray stochastic_with_heureka_reduce
    ( we::type::bytearray partial_resultL_bytearray
    , we::type::bytearray partial_resultR_bytearray
    , we::type::bytearray
    )
{
  std::string const pathL (partial_resultL_bytearray.to_string());
  std::string const pathR (partial_resultR_bytearray.to_string());

  return we::type::bytearray
    { (pathL.size() && pathR.size()) ? ( pathL.size() < pathR.size()
                                       ? pathL
                                       : pathR
                                       )
      : pathL.size() ? pathL
      : pathR.size() ? pathR
      : std::string()
    };
}

extern "C"
  we::type::bytearray stochastic_with_heureka_post_process
    ( unsigned long
    , we::type::bytearray result_bytearray
    , we::type::bytearray
    )
{
  return result_bytearray;
}
