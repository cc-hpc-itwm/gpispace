// Copyright (C) 2010-2016,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/property.fwd.hpp>

#include <gspc/we/type/value/read.hpp>
#include <gspc/we/type/value/serialize.hpp>

#include <gspc/util/xml.fwd.hpp>

#include <functional>
#include <optional>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>

#include <iosfwd>
#include <list>



    namespace gspc::we::type::property
    {
      struct type
      {
      public:
        type();

        value_type const& value() const;
        pnet::type::value::structured_type const& list() const;

        void set (path_type const& path, value_type const&);
        std::optional<std::reference_wrapper<value_type const>> get (path_type const& path) const;
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
        void dump (::gspc::util::xml::xmlstream&, type const&);
      }

      std::ostream& operator<< (std::ostream& s, type const& t);
    }
