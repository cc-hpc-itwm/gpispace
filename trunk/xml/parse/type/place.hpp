// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_PLACE_HPP
#define _XML_PARSE_TYPE_PLACE_HPP

#include <iostream>
#include <string>
#include <vector>

#include <parse/type/token.hpp>

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
        const std::string name;
        const std::string type;
        const maybe<petri_net::capacity_t> capacity;
        std::vector<token> token_vec;

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
        s << "place ("
          << "name = " << p.name
          << ", type = " << p.type
          << ", capacity = " << p.capacity
          ;

        for ( std::vector<token>::const_iterator tok (p.token_vec.begin())
            ; tok != p.token_vec.end()
            ; ++tok
            )
          {
            s << ", token = " << *tok;
          }

        return s << ")";
      }
    }
  }
}

#endif
