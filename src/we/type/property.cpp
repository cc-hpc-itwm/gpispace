// Copyright (C) 2012-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/property.hpp>

#include <gspc/we/type/value/dump.hpp>
#include <gspc/we/type/value/peek.hpp>
#include <gspc/we/type/value/poke.hpp>

#include <gspc/util/xml.hpp>

#include <optional>
#include <boost/utility.hpp>

#include <functional>
#include <iterator>



    namespace gspc::we::type::property
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

      std::optional<std::reference_wrapper<value_type const>>
        type::get (path_type const& path) const
      {
        return pnet::type::value::peek (path.begin(), path.end(), _value);
      }

      bool type::is_true (path_type const& path) const
      {
        auto const value (get (path));

        return !!value && ::boost::get<bool> (value->get());
      }

      namespace dump
      {
        void dump (::gspc::util::xml::xmlstream& s, type const& p)
        {
          pnet::type::value::dump (s, p.value());
        }
      }

      std::ostream& operator << (std::ostream& s, type const& t)
      {
        ::gspc::util::xml::xmlstream xs (s);

        dump::dump (xs, t);

        return s;
      }
    }
