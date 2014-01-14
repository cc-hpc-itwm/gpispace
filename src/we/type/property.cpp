// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <we/type/property.hpp>

#include <we/type/value/path/split.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
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

      std::list<std::pair<key_type, pnet::type::value::value_type> > const&
        type::list() const
      {
        return boost::get<pnet::type::value::structured_type const&> (_value);
      }

      boost::optional<pnet::type::value::value_type>
        type::set ( const path_iterator& pos
                  , const path_iterator& end
                  , const value_type& val
                  )
      {
        boost::optional<pnet::type::value::value_type> old_value
          (pnet::type::value::peek (pos, end, _value));

        pnet::type::value::poke (pos, end, _value, val);

        return old_value;
      }

      boost::optional<pnet::type::value::value_type>
        type::set (const path_type& path, const value_type& val)
      {
        return set (path.begin(), path.end(), val);
      }

      boost::optional<pnet::type::value::value_type>
        type::set (const std::string& path, const value_type& val)
      {
        return set (pnet::type::value::path::split (path), val);
      }

      // ----------------------------------------------------------------- //

      const boost::optional<const value_type&> type::get
        (const path_iterator& pos, const path_iterator& end) const
      {
        boost::optional<const pnet::type::value::value_type&> mapped
          (pnet::type::value::peek (pos, end, _value));

        if (!mapped)
        {
          return boost::none;
        }

        return fhg::util::boost::get_or_none<const value_type&> (*mapped);
      }

      const boost::optional<const value_type&>
        type::get (const path_type& path) const
      {
        return get (path.begin(), path.end());
      }

      const boost::optional<const value_type&>
        type::get (const std::string& path) const
      {
        return get (pnet::type::value::path::split (path));
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
