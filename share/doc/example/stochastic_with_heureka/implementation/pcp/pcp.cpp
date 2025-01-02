// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <interface.hpp>

#include <implementation/pcp/pcp.hpp>

#include <cstddef>
#include <random>
#include <sstream>
#include <vector>

namespace pcp
{
  std::optional<configuration>
    configuration::apply (std::pair<std::string, std::string> const& rule) const
  {
    std::string const top (_top.value_or ("") + rule.first);
    std::string const bot (_bot.value_or ("") + rule.second);

    std::string::const_iterator top_pos (top.cbegin());
    std::string::const_iterator bot_pos (bot.cbegin());

    while (top_pos != top.end() && bot_pos != bot.end() && *top_pos == *bot_pos)
    {
      ++top_pos;
      ++bot_pos;
    }

    return top_pos == top.end() || bot_pos == bot.end()
      ? std::make_optional
        ( configuration
          { top_pos != top.end()
            ? std::make_optional (std::string (top_pos, top.end()))
            : std::nullopt
          , bot_pos != bot.end()
            ? std::make_optional (std::string (bot_pos, bot.end()))
            : std::nullopt
          }
        )
      : std::nullopt
      ;
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
  pcp::configuration configuration (std::nullopt, std::nullopt);

  for (unsigned long i (0); i < n; ++i)
  {
    std::vector<std::pair<char, pcp::configuration>> successors;

    char rule_id ('a');

    for (std::pair<std::string, std::string> rule : pcp.rules())
    {
      if (auto successor {configuration.apply (rule)})
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
