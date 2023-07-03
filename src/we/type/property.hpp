// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/property.fwd.hpp>

#include <we/type/value/read.hpp>
#include <we/type/value/serialize.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <boost/optional/optional_fwd.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>

#include <iosfwd>
#include <list>

namespace we
{
  namespace type
  {
    namespace property
    {
      struct type
      {
      public:
        type();

        value_type const& value() const;
        pnet::type::value::structured_type const& list() const;

        void set (path_type const& path, value_type const&);
        ::boost::optional<value_type const&> get (path_type const& path) const;
        bool is_true (path_type const&) const;

      private:
        value_type _value;

        friend class ::boost::serialization::access;
        template<typename Archive>
        void serialize (Archive& ar, unsigned int)
        {
          ar & _value;
        }
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, type const&);
      }

      std::ostream& operator<< (std::ostream& s, type const& t);
    }
  }
}
