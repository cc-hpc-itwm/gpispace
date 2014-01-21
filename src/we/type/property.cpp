// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <we/type/property.hpp>

#include <we/type/value/path/split.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/remove.hpp>
#include <we/type/value/dump.hpp>

#include <fhg/util/boost/variant.hpp>
#include <fhg/util/xml.hpp>

#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <boost/utility.hpp>

namespace we
{
  namespace type
  {
    namespace property
    {
      type::type ()
        : _value (pnet::type::value::structured_type())
      {}

      pnet::type::value::value_type const& type::value() const
      {
        return _value;
      }

      pnet::type::value::structured_type const& type::list() const
      {
        return boost::get<pnet::type::value::structured_type const&> (_value);
      }

      void type::set ( const path_type::const_iterator& pos
                     , const path_type::const_iterator& end
                     , const value_type& val
                     )
      {
        pnet::type::value::poke (pos, end, _value, val);
      }

      void type::set (const path_type& path, const value_type& val)
      {
        return set (path.begin(), path.end(), val);
      }

      void type::set (const std::string& path, const value_type& val)
      {
        return set (pnet::type::value::path::split (path), val);
      }

      boost::optional<const value_type&> type::get
        ( const path_type::const_iterator& pos
        , const path_type::const_iterator& end
        ) const
      {
        boost::optional<const pnet::type::value::value_type&> mapped
          (pnet::type::value::peek (pos, end, _value));

        if (!mapped)
        {
          return boost::none;
        }

        return fhg::util::boost::get_or_none<const value_type&> (*mapped);
      }

      boost::optional<const value_type&>
        type::get (const path_type& path) const
      {
        return get (path.begin(), path.end());
      }

      boost::optional<const value_type&>
        type::get (const std::string& path) const
      {
        return get (pnet::type::value::path::split (path));
      }

      void type::del ( const path_type::const_iterator& pos
                     , const path_type::const_iterator& end
                     )
      {
        pnet::type::value::remove (pos, end, _value);
      }

      void type::del (const path_type& path)
      {
        return del (path.begin(), path.end());
      }

      void type::del (const std::string& path)
      {
        return del (pnet::type::value::path::split (path));
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, const type& p)
        {
          pnet::type::value::dump (s, p.value());
        }
      }

      std::ostream& operator << (std::ostream& s, const type& t)
      {
        ::fhg::util::xml::xmlstream xs (s);

        dump::dump (xs, t);

        return s;
      }
    }
  }
}
