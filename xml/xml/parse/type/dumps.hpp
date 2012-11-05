// bernd.loerwald@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_DUMPS_HPP
#define _XML_PARSE_TYPE_DUMPS_HPP

//! \note To prevent including nearly everything, it is required to
//! include dumps.hpp AFTER including types used when calling dumps().

#include <fhg/util/xml.hpp>

#include <xml/parse/id/mapper.hpp>

// #include <xml/parse/type/function.hpp>
// #include <xml/parse/type/place.hpp>
// #include <xml/parse/type/place_map.hpp>
// #include <xml/parse/type/port.hpp>
// #include <xml/parse/type/specialize.hpp>
// #include <xml/parse/type/struct.hpp>
// #include <xml/parse/type/template.hpp>
// #include <xml/parse/type/transition.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      namespace dump
      {
        template<typename IT>
        void dumps ( ::fhg::util::xml::xmlstream & s
                   , IT pos
                   , const IT & end
                   )
        {
          for (; pos != end; ++pos)
          {
            ::xml::parse::type::dump::dump (s, *pos);
          }
        }

        template<typename IT, typename T>
        void dumps ( ::fhg::util::xml::xmlstream & s
                   , IT pos
                   , const IT & end
                   , const T & x
                   )
        {
          for (; pos != end; ++pos)
          {
            ::xml::parse::type::dump::dump (s, *pos, x);
          }
        }

        template<typename Container>
        void dumps ( ::fhg::util::xml::xmlstream& s
                   , Container container
                   )
        {
          BOOST_FOREACH (const typename Container::value_type val, container)
          {
            ::xml::parse::type::dump::dump (s, val);
          }
        }

        template<typename Container, typename T>
        void dumps ( ::fhg::util::xml::xmlstream& s
                   , Container container
                   , const T& x
                   )
        {
          BOOST_FOREACH (const typename Container::value_type val, container)
          {
            ::xml::parse::type::dump::dump (s, val, x);
          }
        }

        template<typename Container>
        void dumps ( ::fhg::util::xml::xmlstream& s
                   , Container container
                   , ::xml::parse::id::mapper* id_mapper
                   )
        {
          BOOST_FOREACH (const typename Container::value_type val, container)
            {
              ::xml::parse::type::dump::dump (s, *id_mapper->get (val));
            }
        }

        template<typename Container, typename T>
        void dumps ( ::fhg::util::xml::xmlstream& s
                   , Container container
                   , ::xml::parse::id::mapper* id_mapper
                   , const T& x
                   )
        {
          BOOST_FOREACH (const typename Container::value_type val, container)
            {
              ::xml::parse::type::dump::dump (s, *id_mapper->get (val), x);
            }
        }
      }
    }
  }
}

#endif
