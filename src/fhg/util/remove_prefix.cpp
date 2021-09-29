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

#include <fhg/util/remove_prefix.hpp>

#include <boost/format.hpp>

namespace fhg
{
  namespace util
  {
    remove_prefix_failed::remove_prefix_failed ( std::string word
                                               , std::string prefix
                                               )
      : std::runtime_error
        ( ( boost::format
            ("remove_prefix failed, rest: prefix = %1%, word = %2%")
          % word
          % prefix
          ).str()
        )
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
