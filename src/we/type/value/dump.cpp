#include <we/type/value/dump.hpp>
#include <we/type/value/show.hpp>

#include <fhg/util/xml.hpp>

#include <boost/format.hpp>

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
                       , unsigned int depth
                       , boost::optional<std::string const&> parent = boost::none
                       )
            : _os (os)
            , _depth (depth)
            , _parent (parent)
          {}

          fhg::util::xml::xmlstream& operator() (const structured_type& m) const
          {
            if (_parent)
            {
              _os.open ("properties");
              _os.attr ("name", *_parent);
            }

            for (std::pair<std::string, value_type> const& kv : m)
            {
              boost::apply_visitor
                (visitor_dump (_os, _depth + 1, kv.first), kv.second);
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
              throw std::runtime_error
                ( ( boost::format ("cannot dump the plain value '%1%'")
                  % show (value)
                  ).str()
                );
            }
            if (_depth < 2)
            {
              throw std::runtime_error
                ( (boost::format ("cannot dump the single level property"
                                 " with key '%1%' and value '%2%'"
                                 ) % *_parent % show (value)
                  ).str()
                );
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
          unsigned int _depth;
          boost::optional<std::string const&> _parent;
        };
      }

      fhg::util::xml::xmlstream& dump ( fhg::util::xml::xmlstream& os
                                      , const value_type& value
                                      )
      {
        return boost::apply_visitor (visitor_dump (os, 0), value);
      }
    }
  }
}
