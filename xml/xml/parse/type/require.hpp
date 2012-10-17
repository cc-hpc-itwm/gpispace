// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_REQUIRE_HPP
#define _XML_PARSE_TYPE_REQUIRE_HPP

#include <xml/parse/util/id_type.hpp>

#include <fhg/util/xml.fwd.hpp>

#include <string>

#include <boost/unordered_map.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::string require_key_type;

      struct requirements_type
      {
      private:
        typedef boost::unordered_map<require_key_type,bool> map_type;

      public:
        requirements_type();

#ifdef BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND
        requirements_type & operator= (requirements_type const & rhs);
#endif // BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND

        void set ( const require_key_type & key
                 , const bool & mandatory = true
                 );

        typedef map_type::const_iterator const_iterator;

        const_iterator begin () const;
        const_iterator end () const;

        void join (const requirements_type& reqs);

      private:
        map_type _map;
      };

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const requirements_type & cs
                  );
      }
    }
  }
}

#endif
