// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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
          ( const state::type&
          , const boost::filesystem::path&
          , std::function<bool (std::string const&, std::string const&)>
          = [](std::string const& l, std::string const& r) { return l == r; }
          );
        ~check_no_change_fstream();
        void commit() const;

        template<typename T> check_no_change_fstream& operator<< (const T& x)
        {
          _oss << x; return *this;
        }
        check_no_change_fstream& operator<< (std::ostream& (*)(std::ostream&));

      private:
        const state::type& _state;
        const boost::filesystem::path _file;
        std::function<bool (std::string const&, std::string const&)> _equal;
        std::ostringstream _oss;

        void write() const;
      };
    }
  }
}
