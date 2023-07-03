// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/property.hpp>

#include <we/type/value/dump.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>

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
        return ::boost::get<pnet::type::value::structured_type> (_value);
      }

      void type::set (path_type const& path, value_type const& val)
      {
        pnet::type::value::poke (path.begin(), path.end(), _value, val);
      }

      ::boost::optional<value_type const&>
        type::get (path_type const& path) const
      {
        return pnet::type::value::peek (path.begin(), path.end(), _value);
      }

      bool type::is_true (path_type const& path) const
      {
        auto const value (get (path));

        return !!value && ::boost::get<bool> (*value);
      }

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream& s, type const& p)
        {
          pnet::type::value::dump (s, p.value());
        }
      }

      std::ostream& operator << (std::ostream& s, type const& t)
      {
        ::fhg::util::xml::xmlstream xs (s);

        dump::dump (xs, t);

        return s;
      }
    }
  }
}
