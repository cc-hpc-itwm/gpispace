#pragma once

#include <we/type/property.fwd.hpp>

#include <we/type/value/read.hpp>
#include <we/type/value/serialize.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/optional/optional_fwd.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>

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

        void set (const path_type& path, const value_type&);
        boost::optional<const value_type&> get (const path_type& path) const;
        bool is_true (path_type const&) const;

      private:
        value_type _value;

        friend class boost::serialization::access;
        template<typename Archive>
        void serialize (Archive& ar, const unsigned int)
        {
          ar & _value;
        }
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const type&);
      }

      std::ostream& operator<< (std::ostream& s, const type& t);
    }
  }
}
