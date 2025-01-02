// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fstream>
#include <sstream>

#include <xml/parse/state.hpp>

#include <boost/filesystem.hpp>

#include <xml/parse/error.hpp>

#include <functional>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      class check_no_change_fstream
      {
      public:
        explicit check_no_change_fstream
          ( state::type const&
          , ::boost::filesystem::path const&
          , std::function<bool (std::string const&, std::string const&)>
          = [](std::string const& l, std::string const& r) { return l == r; }
          );
        ~check_no_change_fstream();
        check_no_change_fstream (check_no_change_fstream const&) = delete;
        check_no_change_fstream (check_no_change_fstream&&) = delete;
        check_no_change_fstream& operator= (check_no_change_fstream const&) = delete;
        check_no_change_fstream& operator= (check_no_change_fstream&&) = delete;

        void commit() const;

        template<typename T> check_no_change_fstream& operator<< (T const& x)
        {
          _oss << x; return *this;
        }
        check_no_change_fstream& operator<< (std::ostream& (*)(std::ostream&));

      private:
        state::type const& _state;
        const ::boost::filesystem::path _file;
        std::function<bool (std::string const&, std::string const&)> _equal;
        std::ostringstream _oss;

        void write() const;
      };
    }
  }
}
