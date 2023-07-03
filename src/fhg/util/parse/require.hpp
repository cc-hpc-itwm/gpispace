// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fhg/util/parse/position.hpp>

#include <functional>

namespace fhg
{
  namespace util
  {
    namespace parse
    {
      namespace require
      {
        //! \note require the given value or throw
        void require (position&, char const&);
        void require (position&, std::string const&);

        //! \note skip all white space characters
        void skip_spaces (position&);

        //! \note return the next character, throws when end()
        char plain_character (position&);

        //! \note return all characters until <until>, while
        //! 'escape''until' is ignored. 'until' will be consumed.
        std::string plain_string
          (position&, char until, char escape = '\\');

        //! \note a c-style identifier ([a-zA-Z_][a-zA-Z_0-9]*)
        std::string identifier (position&);

        //! \todo eliminate the skip_space in all the functions below

        //! \note skip spaces, require what
        template<typename T> void token (position& pos, T x)
        {
          skip_spaces (pos);
          require::require (pos, x);
        }

        //! \note skip spaces, single-tick-character ('x')
        char character (position&);

        //! \note skip spaces, double-tick-string ("foobar")
        std::string string (position&);

        //! \note skip spaces, '0' or 'false' or 'no' or 'off' == false,
        //!                    '1' or 'on' or 'true' or 'yes' == true
        bool boolean (position&);

        //! \note expect a list, calling given function for all elements.
        //!       <open>[<skip? *><list_element><skip? *><sep>]*<close>
        //!       the function is expected to consume everything up to
        //!       the list seperator.
        void list ( position&
                  , char open, char sep, char close
                  , std::function<void (position&)> const&
                  , bool skip_space_before_element = true
                  , bool skip_space_after_element = true
                  );

        //! \note everything until pos.end()
        std::string rest (position&);
      }
    }
  }
}
