// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _WE_TYPE_PROPERTY_HPP
#define _WE_TYPE_PROPERTY_HPP

#include <we/type/property.fwd.hpp>

#include <fhg/util/backtracing_exception.hpp>
#include <fhg/util/xml.fwd.hpp>

#include <list>

#include <boost/lexical_cast.hpp>
#include <boost/optional/optional_fwd.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/unordered_map.hpp>
#include <boost/variant.hpp>

namespace we
{
  namespace type
  {
    namespace property
    {
      typedef path_type::const_iterator path_iterator;

      // ******************************************************************* //

      namespace exception
      {
        class empty_path : public std::runtime_error
        {
        public:
          empty_path (const std::string& pre);
        };

        class not_a_map : public std::runtime_error
        {
        public:
          not_a_map (const std::string& pre);
        };
      }

      struct type
      {
      private:
        boost::unordered_map<key_type, mapped_type> map;

        friend class boost::serialization::access;
        template<typename Archive>
        void serialize (Archive& ar, const unsigned int)
        {
          ar & BOOST_SERIALIZATION_NVP(map);
        }

        // ----------------------------------------------------------------- //

        boost::optional<const mapped_type&>
          get ( const path_iterator& pos
              , const path_iterator& end
              , const path_iterator& zero
              ) const;

        const boost::optional<const value_type&> get
          (const path_iterator& pos, const path_iterator& end) const;

        boost::optional<mapped_type> set ( const path_iterator& pos
                                         , const path_iterator& end
                                         , const value_type& val
                                         );

      public:
        type();

        std::list<std::pair<key_type, mapped_type> > list() const;

        boost::optional<mapped_type>
          set (const path_type& path, const value_type& val);
        boost::optional<mapped_type>
          set (const std::string& path, const value_type& val);

        const boost::optional<const value_type&>
          get (const path_type& path) const;
        const boost::optional<const value_type&>
          get (const std::string& path) const;
      };

      namespace dump
      {
        void dump (::fhg::util::xml::xmlstream&, const type&);
      }

      std::ostream& operator<< (std::ostream& s, const type& t);

      // ******************************************************************* //

      namespace traverse
      {
        typedef std::pair<path_type, value_type> pair_type;
        typedef std::list<pair_type> stack_type;

        stack_type dfs (const type& t);
      }
    }
  }
}

#endif
