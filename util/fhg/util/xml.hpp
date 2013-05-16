// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_UTIL_XML_HPP
#define _FHG_UTIL_XML_HPP 1

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
          tag (const std::string& tag);

          const std::string& string() const;
          const bool& has_content() const;
          const bool& has_text_content() const;
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

        void open (const std::string& tag);
        void close();

        template<typename Key, typename Val>
        void attr (const Key& key, const Val& val) const
        {
          assert_nonempty ("attr");

          _s << " " << key << "="
             << "\"" << std::boolalpha << val << std::noboolalpha << "\""
            ;
        }

        template<typename Key, typename Val>
          void attr (const Key& key, const boost::optional<Val>& val) const
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
        std::ostream& _s;
        std::stack<detail::tag> _tag;

        void endl() const;
        void newline() const;

        void assert_nonempty (const std::string& msg) const;

        void add_content (const bool text);
      };
    }
  }
}

#endif
