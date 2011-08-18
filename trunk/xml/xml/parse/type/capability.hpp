// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_CAPABILITY_HPP
#define _XML_PARSE_TYPE_CAPABILITY_HPP 1

#include <string>

#include <boost/unordered_map.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      typedef std::string capability_key_type;

      struct capabilities_type
      {
      private:
        typedef boost::unordered_map<capability_key_type,bool> map_type;

        map_type map;

      public:
        capabilities_type () : map (0) {}

        void set ( const capability_key_type & key
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

        typedef map_type::const_iterator iterator;

        iterator begin () const { return map.begin(); }
        iterator end () const { return map.end(); }
      };

      namespace dump
      {
        inline void dump (xml_util::xmlstream & s, const capabilities_type & cs)
        {
          for ( capabilities_type::iterator cap (cs.begin())
              ; cap != cs.end()
              ; ++cap
              )
            {
              s.open ("capability");
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
