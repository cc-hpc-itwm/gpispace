#include <xml/parse/type/specialize.hpp>

#include <xml/parse/type/net.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      specialize_type::specialize_type ( const util::position_type& pod
                                       , const std::string& name
                                       , const std::string& use_
                                       , const type_map_type& type_map_
                                       , const type_get_type& type_get_
                                       )
        : with_position_of_definition (pod)
        , _name (name)
        , use (use_)
        , type_map (type_map_)
        , type_get (type_get_)
      {}

      const std::string& specialize_type::name() const
      {
        return _name;
      }

      const specialize_type::unique_key_type&
        specialize_type::unique_key() const
      {
        return name();
      }

      void split_structs ( const xml::parse::structure_type_util::set_type & global
                         , structs_type & child_structs
                         , structs_type & parent_structs
                         , const type_get_type & type_get
                         , const state::type & state
                         )
      {
        namespace st = xml::parse::structure_type_util;

        const st::set_type known_structs
          ( st::join ( global
                     , st::join ( st::make (parent_structs, state)
                                , st::make (child_structs, state)
                                )
                     )
          );

        structs_type structs (child_structs);

        child_structs.clear();

        for (structure_type& s : structs)
        {
          const type_get_type::const_iterator pos (type_get.find (s.name()));

          if (pos == type_get.end())
          {
            child_structs.push_back (s);
          }
          else
          {
            s.resolve (known_structs);

            parent_structs.push_back (s);
          }
        }
      }

      namespace dump
      {
        void dump ( ::fhg::util::xml::xmlstream & s
                  , const specialize_type & sp
                  )
        {
          s.open ("specialize");
          s.attr ("name", sp.name());
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
      }
    }
  }
}
