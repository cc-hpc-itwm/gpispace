// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/dump.hpp>
#include <we/type/value/show.hpp>

#include <fhg/util/xml.hpp>

#include <boost/foreach.hpp>

#include <sstream>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace
      {
        class visitor_dump
          : public boost::static_visitor<fhg::util::xml::xmlstream&>
        {
        public:
          visitor_dump ( fhg::util::xml::xmlstream& os
                       , boost::optional<std::string const&> parent = boost::none
                       )
            : _os (os)
            , _parent (parent)
          {}

          fhg::util::xml::xmlstream& operator() (const structured_type& m) const
          {
            typedef std::pair<std::string, value_type> kv_type;

            if (_parent)
            {
              _os.open ("properties");
              _os.attr ("name", *_parent);
            }

            BOOST_FOREACH (kv_type const& kv, m)
            {
              boost::apply_visitor (visitor_dump (_os, kv.first), kv.second);
            }

            if (_parent)
            {
              _os.close();
            }

            return _os;
          }
          template<typename T>
          fhg::util::xml::xmlstream& operator() (const T& value) const
          {
            if (!_parent)
            {
              throw std::runtime_error ("cannot dump a plain value");
            }

            _os.open ("property");
            _os.attr ("key", *_parent);

            std::ostringstream oss;
            oss << show (value);
            _os.content (oss.str());

            _os.close();

            return _os;
          }

        private:
          fhg::util::xml::xmlstream& _os;
          boost::optional<std::string const&> _parent;
        };
      }

      fhg::util::xml::xmlstream& dump ( fhg::util::xml::xmlstream& os
                                      , const value_type& value
                                      )
      {
        return boost::apply_visitor (visitor_dump (os), value);
      }
    }
  }
}
