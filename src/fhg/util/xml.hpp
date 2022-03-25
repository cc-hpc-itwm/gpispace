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

#pragma once

#include <fhg/util/xml.fwd.hpp>

#include <iostream>
#include <string>
#include <stack>

#include <boost/optional.hpp>

namespace fhg
{
  namespace util
  {
    namespace xml
    {
      namespace detail
      {
        struct tag
        {
        public:
          tag (std::string const& tag);

          std::string const& string() const;
          bool const& has_content() const;
          bool const& has_text_content() const;
          void has_content (bool x);
          void has_text_content (bool x);

        private:
          const std::string _tag;
          bool _has_content;
          bool _has_text_content;
        };
      }

      class xmlstream
      {
      public:
        xmlstream (std::ostream& s);
        ~xmlstream();
        xmlstream (xmlstream const&) = delete;
        xmlstream (xmlstream&&) = delete;
        xmlstream& operator= (xmlstream const&) = delete;
        xmlstream& operator= (xmlstream&&) = delete;

        void open (std::string const& tag);
        void close();

        template<typename Key, typename Val>
        void attr (Key const& key, Val const& val) const
        {
          assert_nonempty ("attr");

          _s << " " << key << "="
             << "\"" << std::boolalpha << val << std::noboolalpha << "\""
            ;
        }

        template<typename Key, typename Val>
          void attr (Key const& key, ::boost::optional<Val> const& val) const
        {
          if (val)
          {
            attr<Key, Val> (key, *val);
          }
        }

        template<typename Key>
          void attr (Key const&, ::boost::none_t const&) const
        {
        }

        template<typename T>
        void content (T x)
        {
          assert_nonempty ("content");

          add_content (true);

          _s << x;
        }

      private:
        std::ostream& _s;
        std::stack<detail::tag> _tag;

        void endl() const;
        void newline() const;

        void assert_nonempty (std::string const& msg) const;

        void add_content (bool text);
      };
    }
  }
}
