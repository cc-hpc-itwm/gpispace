// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <ostream>
#include <streambuf>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      class redirect : public std::streambuf
      {
      public:
        redirect ( std::ostream& os
                 , std::function<void (std::string const&)> line
                 );

        ~redirect() override;

        int_type overflow (int_type c) override;

        redirect (redirect const&) = delete;
        redirect (redirect&&) = delete;
        redirect& operator= (redirect const&) = delete;
        redirect& operator= (redirect&&) = delete;

      private:
        std::ostream& _os;
        std::function<void (std::string const&)> _line;
        std::string _buffer;
        std::streambuf* _streambuf;
      };

      class prepend_line : public redirect
      {
      public:
        prepend_line ( std::ostream& os
                     , std::function<std::string (std::string const&)> prefix
                     );
        prepend_line ( std::ostream& os
                     , std::function<std::string ()> prefix
                     );
        prepend_line (std::ostream& os, std::string prefix);
      };
    }
  }
}
