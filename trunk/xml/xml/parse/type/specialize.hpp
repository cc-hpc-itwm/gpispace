// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_TYPE_SPECIALIZE_HPP
#define _XML_PARSE_TYPE_SPECIALIZE_HPP

#include <xml/parse/types.hpp>

#include <iostream>

#include <boost/unordered_map.hpp>

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
        int level;
        type_map_type type_map;
        type_get_type type_get;
      };

      inline void
      split_structs ( const xml::parse::struct_t::set_type & global
                    , struct_vec_type & child_structs
                    , struct_vec_type & parent_structs
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

        struct_vec_type structs (child_structs);

        child_structs.clear();

        for ( struct_vec_type::iterator s (structs.begin())
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
                         , struct_vec_type & structs
                         , const state::type & state
                         )
      {
        for ( struct_vec_type::iterator s (structs.begin())
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

      inline std::ostream &
      operator << (std::ostream & s, const specialize_type & sp)
      {
        s << level (sp.level) << "specialize (" << std::endl;
        s << level (sp.level + 1) << "name = " << sp.name << std::endl;
        s << level (sp.level + 1) << "use = " << sp.use << std::endl;

        s << level (sp.level + 1) << "type-map = " << std::endl;

        for ( type_map_type::const_iterator pos (sp.type_map.begin())
            ; pos != sp.type_map.end()
            ; ++pos
            )
          {
            s << level (sp.level + 2)
              << pos->first << " => " << pos->second
              << std::endl
              ;
          }

        s << level (sp.level + 1) << "type-get = " << std::endl;

        for ( type_get_type::const_iterator pos (sp.type_get.begin())
            ; pos != sp.type_get.end()
            ; ++pos
            )
          {
            s << level (sp.level + 2) << *pos << std::endl;
          }

        return s << level (sp.level) << ") // specialize";
      }
    }
  }
}

#endif
