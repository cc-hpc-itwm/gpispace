// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    class remove_prefix_failed : public std::runtime_error
    {
    public:
      remove_prefix_failed (std::string word, std::string prefix);

      std::string const& word() const { return _word; }
      std::string const& prefix() const { return _prefix; }
    private:
      const std::string _word;
      const std::string _prefix;
    };

    std::string remove_prefix ( std::string const& prefix
                              , std::string const& word
                              );
  }
}
