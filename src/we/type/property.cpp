// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <we/type/property.hpp>

#include <we/type/value/path/split.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/positions.hpp>
#include <we/type/value/dump.hpp>

#include <fhg/util/boost/variant.hpp>
#include <fhg/util/split.hpp>
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
      namespace exception
      {
        empty_path::empty_path (const std::string& pre)
          : std::runtime_error (pre + ": empty path")
        {}

        not_a_map::not_a_map (const std::string& pre)
          : std::runtime_error (pre + ": not a map")
        {}
      }

      type::type ()
        : _value (pnet::type::value::structured_type())
      {}

      std::list<std::pair<key_type, mapped_type> > const& type::list() const
      {
        return boost::get<pnet::type::value::structured_type const&> (_value);
      }

      boost::optional<mapped_type> type::set ( const path_iterator& pos
                                             , const path_iterator& end
                                             , const value_type& val
                                             )
      {
        boost::optional<mapped_type> old_value
          (pnet::type::value::peek (pos, end, _value));

        pnet::type::value::poke (pos, end, _value, val);

        return old_value;
      }

      boost::optional<mapped_type>
        type::set (const path_type& path, const value_type& val)
      {
        return set (path.begin(), path.end(), val);
      }

      boost::optional<mapped_type>
        type::set (const std::string& path, const value_type& val)
      {
        return set (pnet::type::value::path::split (path), val);
      }

      // ----------------------------------------------------------------- //

      const boost::optional<const value_type&> type::get
        (const path_iterator& pos, const path_iterator& end) const
      {
        boost::optional<const mapped_type&> mapped
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

      // ******************************************************************* //

      namespace traverse
      {
        stack_type dfs (const type& t)
        {
          stack_type s;

          typedef std::pair< std::list<std::string>
                           , pnet::type::value::value_type
                           > position_type;

          BOOST_FOREACH ( position_type const& position
                        , pnet::type::value::positions (t.value())
                        )
          {
            s.push_back
              ( std::make_pair ( position.first
                               , boost::get<std::string> (position.second)
                               )
              );
          }

          return s;
        }
      }
    }
  }
}
