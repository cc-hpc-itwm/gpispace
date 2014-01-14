// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _WE_TYPE_PROPERTY_HPP
#define _WE_TYPE_PROPERTY_HPP

#include <we/type/property.fwd.hpp>

#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/optional/optional_fwd.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>

#include <list>
#include <sstream>

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

        pnet::type::value::value_type const& value() const
        {
          return _value;
        }
        pnet::type::value::structured_type const& list() const;

        void set (const path_type& path, const value_type& val);
        void set (const std::string& path, const value_type& val);

        const boost::optional<const value_type&>
          get (const path_type& path) const;
        const boost::optional<const value_type&>
          get (const std::string& path) const;

      private:
        pnet::type::value::value_type _value;

        friend class boost::serialization::access;
        template<typename Archive>
        void save (Archive& ar, const unsigned int) const
        {
          std::ostringstream oss;
          oss << pnet::type::value::show (_value);
          std::string const val (oss.str());
          ar & val;
        }
        template<typename Archive>
        void load (Archive& ar, const unsigned int)
        {
          std::string val;
          ar & val;
          _value = pnet::type::value::read (val);
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()

        void set ( const path_type::const_iterator& pos
                 , const path_type::const_iterator& end
                 , const value_type& val
                 );

        const boost::optional<const value_type&> get
          ( const path_type::const_iterator& pos
          , const path_type::const_iterator& end
          ) const;

      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const type&);
      }

      std::ostream& operator<< (std::ostream& s, const type& t);
    }
  }
}

#endif
