// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_REQUIRE_HPP
#define _XML_PARSE_TYPE_REQUIRE_HPP 1

#include <string>

#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>

#include <fhg/util/xml.hpp>

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

        map_type map;

      public:
        requirements_type () : map (0) {}

#ifdef BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND
        requirements_type & operator= (requirements_type const & rhs)
        {
          if (this != &rhs)
          {
            map = rhs.map;
          }
          return *this;
        }
#endif // BOOST_1_48_ASSIGNMENT_OPERATOR_WORKAROUND

        void set ( const require_key_type & key
                 , const bool & mandatory = true
                 )
        {
          map_type::iterator pos (map.find (key));

          if (pos != map.end())
            {
              pos->second |= mandatory;
            }
          else
            {
              map[key] = mandatory;
            }
        }

        typedef map_type::const_iterator const_iterator;

        const_iterator begin () const { return map.begin(); }
        const_iterator end () const { return map.end(); }

        void join (const requirements_type& reqs)
        {
          BOOST_FOREACH (const map_type::value_type& req, reqs)
            {
              set (req.first, req.second);
            }
        }
      };

      namespace dump
      {
        inline void dump ( xml_util::xmlstream & s
                         , const requirements_type & cs
                         )
        {
          for ( requirements_type::const_iterator cap (cs.begin())
              ; cap != cs.end()
              ; ++cap
              )
            {
              s.open ("require");
              s.attr ("key", cap->first);
              s.attr ("mandatory", cap->second);
              s.close ();
            }
        }
      }
    }
  }
}

#endif
