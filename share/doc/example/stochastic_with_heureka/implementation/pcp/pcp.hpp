// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
    configuration ( ::boost::optional<std::string> top
                  , ::boost::optional<std::string> bot
                  )
      : _top (top)
      , _bot (bot)
    {}
    ::boost::optional<configuration> apply
      (std::pair<std::string, std::string> const& rule) const;
    bool is_solution() const
    {
      return !_top && !_bot;
    }
    void clear()
    {
      _top = _bot = ::boost::none;
    }

  private:
    ::boost::optional<std::string> _top;
    ::boost::optional<std::string> _bot;
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
