// bernd.loerwald@itwm.fraunhofer.de

#include <xml/parse/type/specialize.hpp>

#include <xml/parse/id/mapper.hpp>
#include <xml/parse/type/net.hpp>

#include <fhg/util/xml.hpp>

namespace xml
{
  namespace parse
  {
    namespace type
    {
      specialize_type::specialize_type ( ID_CONS_PARAM(specialize)
                                       , PARENT_CONS_PARAM(net)
                                       , const std::string& name
                                       , const std::string& use
                                       , const type_map_type& type_map
                                       , const type_get_type& type_get
                                       , const boost::filesystem::path& path
                                       )
        : ID_INITIALIZE()
        , PARENT_INITIALIZE()
        , _name (name)
        , use (use)
        , type_map (type_map)
        , type_get (type_get)
        , path (path)
      {
        _id_mapper->put (_id, *this);
      }

      const std::string& specialize_type::name() const
      {
        return _name;
      }
      const std::string& specialize_type::name_impl (const std::string& name)
      {
        return _name = name;
      }
      const std::string& specialize_type::name (const std::string& name)
      {
        if (has_parent())
        {
          parent()->rename (make_reference_id(), name);
          return _name;
        }
        return name_impl (name);
      }

      const specialize_type::unique_key_type&
        specialize_type::unique_key() const
      {
        return name();
      }

      id::ref::specialize specialize_type::clone
        ( const boost::optional<parent_id_type>& parent
        , const boost::optional<id::mapper*>& mapper
        ) const
      {
        id::mapper* const new_mapper (mapper.get_value_or (id_mapper()));
        const id_type new_id (new_mapper->next_id());
        return specialize_type
          ( new_id
          , new_mapper
          , parent
          , _name
          , use
          , type_map
          , type_get
          , path
          ).make_reference_id();
      }

      void split_structs ( const xml::parse::structure_type::set_type & global
                         , structs_type & child_structs
                         , structs_type & parent_structs
                         , const type_get_type & type_get
                         , const state::type & state
                         )
      {
        namespace st = xml::parse::structure_type;

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
            const type_get_type::const_iterator pos (type_get.find (s->name()));

            if (pos == type_get.end())
              {
                child_structs.push_back (*s);
              }
            else
              {
                boost::apply_visitor ( st::resolve (known_structs, s->path())
                                     , s->signature()
                                     );

                parent_structs.push_back (*s);
              }
          }
      }

      void specialize_structs ( const type_map_type & map
                              , structs_type & structs
                              , const state::type & state
                              )
      {
        for ( structs_type::iterator s (structs.begin())
            ; s != structs.end()
            ; ++s
            )
        {
          s->signature
            ( boost::apply_visitor
              ( xml::parse::structure_type::specialize (map)
              , s->signature()
              )
            );

          type_map_type::const_iterator pos (map.find (s->name()));

          if (pos != map.end())
          {
            s->name (pos->second);
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
