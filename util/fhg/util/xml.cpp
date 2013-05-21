// bernd.loerwald@itwm.fraunhofer.de

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
        tag::tag (const std::string& tag)
          : _tag (tag)
          , _has_content (false)
          , _has_text_content (false)
        {}

        const std::string& tag::string() const { return _tag; }
        const bool& tag::has_content() const { return _has_content; }
        const bool& tag::has_text_content() const { return _has_text_content; }
        void tag::has_content (bool x) { _has_content = x; }
        void tag::has_text_content (bool x) { _has_text_content = x; }
      }

      xmlstream::xmlstream (std::ostream& s) : _s (s), _tag() {}
      xmlstream::~xmlstream() { endl(); }

      void xmlstream::open (const std::string& tag)
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

      void xmlstream::assert_nonempty (const std::string& msg) const
      {
        if (_tag.empty())
        {
          throw std::runtime_error (msg + ": tag stack empty");
        }
      }

      void xmlstream::add_content (const bool text)
      {
        if (!_tag.top().has_content())
        {
          _s << ">";

          _tag.top().has_content(true);
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
