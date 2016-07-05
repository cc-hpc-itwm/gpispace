// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <we/type/property.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>
#include <we/type/value/dump.hpp>

#include <fhg/util/boost/variant.hpp>
#include <fhg/util/xml.hpp>

#include <boost/optional.hpp>
#include <boost/utility.hpp>

#include <functional>
#include <iterator>

namespace we
{
  namespace type
  {
    namespace property
    {
      type::type ()
        : _value (pnet::type::value::structured_type())
      {}

      value_type const& type::value() const
      {
        return _value;
      }

      pnet::type::value::structured_type const& type::list() const
      {
        return boost::get<pnet::type::value::structured_type> (_value);
      }

      void type::set (const path_type& path, const value_type& val)
      {
        pnet::type::value::poke (path.begin(), path.end(), _value, val);
      }

      boost::optional<const value_type&>
        type::get (const path_type& path) const
      {
        return pnet::type::value::peek (path.begin(), path.end(), _value);
      }

      bool type::is_true (path_type const& path) const
      {
        auto const value (get (path));

        return !!value && boost::get<bool> (*value);
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
