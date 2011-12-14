// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_SPECIALIZE_HPP
#define _XML_PARSE_TYPE_SPECIALIZE_HPP

#include <xml/parse/types.hpp>

#include <iostream>

#include <boost/unordered_map.hpp>

#include <fhg/util/xml.hpp>

namespace xml_util = ::fhg::util::xml;

namespace xml
{
  namespace parse
  {
    namespace type
    {
      struct specialize_type
      {
        std::string name;
        std::string use;
        type_map_type type_map;
        type_get_type type_get;
      };

      inline void
      split_structs ( const xml::parse::struct_t::set_type & global
                    , structs_type & child_structs
                    , structs_type & parent_structs
                    , const type_get_type & type_get
                    , const state::type & state
                    )
      {
        namespace st = xml::parse::struct_t;

        const st::set_type known_structs
          ( st::join ( global
                     , st::join ( st::make (parent_structs)
                                , st::make (child_structs)
                                , st::forbidden_type()
                                , state
                                )
                     , st::forbidden_type()
                     , state
                     )
          );

        structs_type structs (child_structs);

        child_structs.clear();

        for ( structs_type::iterator s (structs.begin())
            ; s != structs.end()
            ; ++s
            )
          {
            const type_get_type::const_iterator pos (type_get.find (s->name));

            if (pos == type_get.end())
              {
                child_structs.push_back (*s);
              }
            else
              {
                boost::apply_visitor ( st::resolve (known_structs, s->path)
                                     , s->sig
                                     );

                parent_structs.push_back (*s);
              }
          }
      }

      inline void
      specialize_structs ( const type_map_type & map
                         , structs_type & structs
                         , const state::type & state
                         )
      {
        for ( structs_type::iterator s (structs.begin())
            ; s != structs.end()
            ; ++s
            )
          {
            s->sig = boost::apply_visitor
              ( xml::parse::struct_t::specialize (map, state)
              , s->sig
              );

            type_map_type::const_iterator pos (map.find (s->name));

            if (pos != map.end())
              {
                s->name = pos->second;
              }
          }
      }

      namespace dump
      {
        inline void dump ( xml_util::xmlstream & s
                         , const specialize_type & sp
                         )
        {
          s.open ("specialize");
          s.attr ("name", sp.name);
          s.attr ("use", sp.use);

          for ( type_map_type::const_iterator tm (sp.type_map.begin())
              ; tm != sp.type_map.end()
              ; ++tm
              )
            {
              s.open ("type-map");
              s.attr ("replace", tm->first);
              s.attr ("with", tm->second);
              s.close ();
            }

          for ( type_get_type::const_iterator tg (sp.type_get.begin())
              ; tg != sp.type_get.end()
              ; ++tg
              )
            {
              s.open ("type-get");
              s.attr ("name", *tg);
              s.close ();
            }

          s.close ();
        }
      } // namespace dump
    }
  }
}

#endif
