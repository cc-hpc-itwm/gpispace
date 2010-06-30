// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_HPP
#define _XML_PARSE_TYPE_PLACE_HPP

#include <iostream>
#include <string>
#include <vector>

#include <parse/type/token.hpp>
#include <parse/types.hpp>

#include <parse/util/maybe.hpp>

#include <we/type/id.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct place
      {
      public:
        std::string name;
        std::string type;
        maybe<petri_net::capacity_t> capacity;
        std::vector<token> token_vec;
        int level;

        place ( const std::string & _name
              , const std::string & _type
              , const maybe<petri_net::capacity_t> _capacity
              )
          : name (_name)
          , type (_type)
          , capacity (_capacity)
          , token_vec ()
        {}

        void push_token (const token & t)
        {
          token_vec.push_back (t);
        }
      };

      std::ostream & operator << (std::ostream & s, const place & p)
      {
        s << level(p.level)  << "place (" << std::endl;
        s << level(p.level+1) << "name = " << p.name << std::endl;
        s << level(p.level+1) << "type = " << p.type << std::endl;
        s << level(p.level+1) << "capacity = " << p.capacity << std::endl;
          ;

        for ( std::vector<token>::const_iterator tok (p.token_vec.begin())
            ; tok != p.token_vec.end()
            ; ++tok
            )
          {
            s << level(p.level+1) << "token = " << *tok << std::endl;
          }

        return s << level(p.level) << ") // place";
      }
    }
  }
}

#endif
