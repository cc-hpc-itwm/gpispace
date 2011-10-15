// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_XML_HPP
#define _FHG_UTIL_XML_HPP 1

#include <fhg/util/maybe.hpp>

#include <iostream>
#include <string>
#include <stack>
#include <stdexcept>

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
          tag (const std::string & tag)
            : _tag (tag)
            , _has_content (false)
          {}

          const std::string & string () const { return _tag; }
          const bool & has_content () const { return _has_content; }
          void has_content (bool x) { _has_content = x; }

        private:
          const std::string _tag;
          bool _has_content;
        };

        typedef std::stack<tag> tag_stack;
      }

      class xmlstream
      {
      public:
        xmlstream (std::ostream & s) : _s (s), _tag() {}
        ~xmlstream () { endl(); }

        void open (const std::string & tag)
        {
          if (!_tag.empty())
            {
              add_content();
            }

          _s << "<" << tag;

          _tag.push (detail::tag (tag));
        }

        void close ()
        {
          assert_nonempty ("close");

          const detail::tag tag (_tag.top());

          _tag.pop();

          if (tag.has_content())
            {
              newline();

              _s << "</" << tag.string() << ">";
            }
          else
            {
              _s << "/>";
            }
        }

        template<typename Key, typename Val>
        void attr (const Key & key, const Val & val) const
        {
          assert_nonempty ("attr");

          _s << " " << key << "="
             << "\"" << std::boolalpha << val << std::noboolalpha << "\""
            ;
        }

        template<typename Key, typename Val>
        void attr (const Key & key, const fhg::util::maybe<Val> & val) const
        {
          if (val.isJust())
            {
              attr<Key, Val> (key, *val);
            }
        }

        template<typename T>
        void content (T x)
        {
          assert_nonempty ("content");

          add_content();

          _s << x;
        }

      private:
        std::ostream & _s;
        detail::tag_stack _tag;

        void level () const
        {
          for (detail::tag_stack::size_type i (0); i < _tag.size(); ++i)
            {
              _s << "  ";
            }
        }

        void endl () const { _s << std::endl; }
        void newline () const { endl(); level(); }

        void assert_nonempty (const std::string & msg) const
        {
          if (_tag.empty())
            {
              throw std::runtime_error (msg + ": tag stack empty");
            }
        }

        void set_content ()
        {
          if (!_tag.top().has_content())
            {
              _s << ">";

              _tag.top().has_content(true);
            }
        }

        void add_content ()
        {
          set_content ();

          newline ();
        }
      };
    }
  }
}

#endif
