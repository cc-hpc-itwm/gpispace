// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhg/util/xml.hpp>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace xml
    {
      namespace detail
      {
        tag::tag (std::string const& tag_)
          : _tag (tag_)
        {}

        std::string const& tag::string() const { return _tag; }
        bool const& tag::has_content() const { return _has_content; }
        bool const& tag::has_text_content() const { return _has_text_content; }
        void tag::has_content (bool x) { _has_content = x; }
        void tag::has_text_content (bool x) { _has_text_content = x; }
      }

      xmlstream::xmlstream (std::ostream& s) : _s (s), _tag() {}
      xmlstream::~xmlstream() { endl(); }

      void xmlstream::open (std::string const& tag)
      {
        if (!_tag.empty())
        {
          add_content (false);
        }

        _s << "<" << tag;

        _tag.push (detail::tag (tag));
      }

      void xmlstream::close()
      {
        assert_nonempty ("close");

        const detail::tag tag (_tag.top());

        _tag.pop();

        if (tag.has_content())
        {
          if (!tag.has_text_content())
          {
            newline();
          }

          _s << "</" << tag.string() << ">";
        }
        else
        {
          _s << "/>";
        }
      }

      void xmlstream::endl() const
      {
        _s << std::endl;
      }
      void xmlstream::newline() const
      {
        endl();
        _s << std::string (_tag.size() * 2, ' ');
      }

      void xmlstream::assert_nonempty (std::string const& msg) const
      {
        if (_tag.empty())
        {
          throw std::runtime_error (msg + ": tag stack empty");
        }
      }

      void xmlstream::add_content (bool text)
      {
        if (!_tag.top().has_content())
        {
          _s << ">";

          _tag.top().has_content (true);
          if (text)
          {
            _tag.top().has_text_content (true);
          }
        }

        if (!text)
        {
          newline();
        }
      }
    }
  }
}
