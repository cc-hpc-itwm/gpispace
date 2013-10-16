// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/type/link.hpp>

#include <fhg/util/xml.hpp>
#include <fhg/util/parse/error.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      link_type::link_type ( const std::string& href
                           , const boost::optional<std::string>& prefix
                           )
        : _href (href)
        , _prefix (prefix)
      {}
      const std::string
      link_type::link (const by_key_function_type& by_key) const
      {
        std::string p;

        if (_prefix)
        {
          fhg::util::parse::position inp (*_prefix);

          while (!inp.end())
          {
            switch (*inp)
            {
            case '$':
              {
                ++inp;
                fhg::util::parse::require::require (inp, '{');
                bool read_key (false);
                std::string key;

                while (!read_key && !inp.end())
                {
                  switch (*inp)
                  {
                  case '}':
                    ++inp;
                    read_key = true;
                    p += by_key (key);
                    break;
                  default: key += *inp; ++inp; break;
                  }
                }

                if (!read_key)
                {
                  throw fhg::util::parse::error::expected ("}", inp);
                }
              }
              break;
            default: p += *inp; ++inp; break;
            }
          }
        }

        return p + _href;
      }

      const std::string& link_type::href() const
      {
        return _href;
      }
      const boost::optional<std::string>& link_type::prefix() const
      {
        return _prefix;
      }

      bool operator== (const link_type& a, const link_type& b)
      {
        return a.href() == b.href() && a.prefix() == b.prefix();
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const link_type& l)
        {
          s.open ("link");
          s.attr ("href", l.href());
          s.attr ("prefix", l.prefix());
          s.close();
        }
      }
    }
  }
}
