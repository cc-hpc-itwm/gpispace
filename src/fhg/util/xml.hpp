// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <fhg/util/xml.fwd.hpp>

#include <iostream>
#include <stack>
#include <string>

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
          bool _has_content {false};
          bool _has_text_content {false};
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
