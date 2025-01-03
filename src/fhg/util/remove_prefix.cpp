// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/remove_prefix.hpp>

#include <fmt/core.h>

namespace fhg
{
  namespace util
  {
    remove_prefix_failed::remove_prefix_failed ( std::string word
                                               , std::string prefix
                                               )
      : std::runtime_error
        { fmt::format
          ( "remove_prefix failed, rest: prefix = {}, word = {}"
          , word
          , prefix
          )
        }
      , _word (word)
      , _prefix (prefix)
    { }

    std::string remove_prefix ( std::string const& prefix
                              , std::string const& word
                              )
    {
      const std::string::const_iterator end_prefix (prefix.end());
      const std::string::const_iterator end_word (word.end());
      std::string::const_iterator begin_prefix (prefix.begin());
      std::string::const_iterator begin_word (word.begin());

      while (  begin_prefix != end_prefix
            && begin_word != end_word
            && *begin_prefix == *begin_word
            )
        {
          ++begin_prefix;
          ++begin_word;
        }

      if (begin_prefix == end_prefix)
        {
          return std::string (begin_word, end_word);
        }

      throw remove_prefix_failed ( std::string (begin_word, end_word)
                                 , std::string (begin_prefix, end_prefix)
                                 );
    }
  }
}
