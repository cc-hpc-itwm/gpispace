// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_XML_HPP
#define _FHG_UTIL_XML_HPP 1

#include <iostream>
#include <string>
#include <stack>
#include <stdexcept>

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
          tag (const std::string & tag)
            : _tag (tag)
            , _has_content (false)
            , _has_text_content (false)
          {}

          const std::string & string () const { return _tag; }
          const bool & has_content () const { return _has_content; }
          const bool& has_text_content() const { return _has_text_content; }
          void has_content (bool x) { _has_content = x; }
          void has_text_content (bool x) { _has_text_content = x; }

        private:
          const std::string _tag;
          bool _has_content;
          bool _has_text_content;
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
              add_content (false);
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

        template<typename Key, typename Val>
        void attr (const Key & key, const Val & val) const
        {
          assert_nonempty ("attr");

          _s << " " << key << "="
             << "\"" << std::boolalpha << val << std::noboolalpha << "\""
            ;
        }

        template<typename Key, typename Val>
          void attr (const Key & key, const boost::optional<Val> & val) const
        {
          if (val)
            {
              attr<Key, Val> (key, *val);
            }
        }

        template<typename Key>
          void attr (const Key&, const boost::none_t&) const
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

        void set_content (const bool text)
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
        }

        void add_content (const bool text)
        {
          set_content (text);

          if (!text)
          {
            newline();
          }
        }
      };
    }
  }
}

#endif
