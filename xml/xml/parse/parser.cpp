// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/parser.hpp>

#include <xml/parse/state.hpp>

#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/token.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/use.hpp>

#include <fhg/util/read.hpp>
#include <fhg/util/maybe.hpp>
#include <fhg/util/read_bool.hpp>
#include <fhg/util/join.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/bind.hpp>

namespace xml
{
  namespace parse
  {
    // ********************************************************************* //

    namespace
    {
      id::function
      parse_function
        ( std::istream & f
        , state::type & state
        , const boost::optional<type::function_type::id_parent>& parent
        )
      {
        return generic_parse<id::function>
          ( boost::bind (function_type, _1, _2, parent)
          , f
          , state
          , "defun"
          , "parse_function"
          );
      }

      id::tmpl parse_template ( std::istream & f
                              , state::type & state
                              , const id::net& parent
                              )
      {
        return generic_parse<id::tmpl>
          ( boost::bind (tmpl_type, _1, _2, parent)
          , f
          , state
          , "template"
          , "parse_template"
          );
      }

      type::structs_type parse_structs ( std::istream & f
                                       , state::type & state
                                       , const id::function& parent
                                       )
      {
        return generic_parse<type::structs_type>
          ( boost::bind (structs_type, _1, _2, parent)
          , f
          , state
          , "structs"
          , "parse_structs"
          );
      }

      we::type::property::type
      parse_props (std::istream & f, state::type & state)
      {
        return generic_parse<we::type::property::type>
          (property_maps_type, f, state, "props", "parse_props");
      }
    }

    // ********************************************************************* //

    static id::function
    function_include
      ( const std::string & file
      , state::type & state
      , const boost::optional<type::function_type::id_parent>& parent
      )
    {
      return state.generic_include<id::function>
        (boost::bind (parse_function, _1, _2, parent), file);
    }

    static id::tmpl template_include ( const std::string & file
                                     , state::type & state
                                     , const id::net& parent
                                     )
    {
      return state.generic_include<id::tmpl>
        (boost::bind (parse_template, _1, _2, parent), file);
    }

    static type::structs_type structs_include ( const std::string & file
                                              , state::type & state
                                              , const id::function& parent
                                              )
    {
      return state.generic_include<type::structs_type>
        (boost::bind (parse_structs, _1, _2, parent), file);
    }

    static we::type::property::type
    properties_include (const std::string & file, state::type & state)
    {
      return
        state.generic_include<we::type::property::type> (parse_props, file);
    }

    // ********************************************************************* //

    static void
    require_type ( type::requirements_type & requirements
                 , const xml_node_type * node
                 , state::type & state
                 )
    {
      const std::string key
        (required ("require_type", node, "key", state.file_in_progress()));
      const fhg::util::maybe<bool> mmandatory
        ( fhg::util::fmap<std::string, bool>( fhg::util::read_bool
                                            , optional (node, "mandatory")
                                            )
        );
      const bool mandatory (mmandatory ? *mmandatory : true);

      requirements.set (key, mandatory);

      // collect all the requirements for the top level function
      state.set_requirement (key, mandatory);
    }

    // ********************************************************************* //

    static void
    set_type_map ( const xml_node_type * node
                 , const state::type & state
                 , type::type_map_type & map
                 )
    {
      const std::string replace
        (required ("set_type_map", node, "replace", state.file_in_progress()));
      const std::string with
        (required ("set_type_map", node, "with", state.file_in_progress()));

      type::type_map_type::const_iterator old (map.find (replace));

      if (old != map.end())
        {
          if (old->second != with)
            {
              throw error::type_map_mismatch
                (replace, old->second, with, state.file_in_progress());
            }
          else
            {
              state.warn ( warning::type_map_duplicate
                           ( replace
                           , with
                           , state.file_in_progress()
                           )
                         );
              }
          }

      map[replace] = with;
    }

    // ********************************************************************* //

    static void
    set_type_get ( const xml_node_type * node
                 , const state::type & state
                 , type::type_get_type & set
                 )
    {
      const std::string name
        (required ("set_type_get", node, "name", state.file_in_progress()));

      type::type_get_type::const_iterator old (set.find (name));

      if (old != set.end())
        {
          state.warn ( warning::type_get_duplicate ( name
                                                   , state.file_in_progress()
                                                   )
                     );
        }

      set.insert (name);
    }

    // ********************************************************************* //

    type::connect_type
    connect_type ( const xml_node_type * node
                 , state::type & state
                 , const id::transition& parent
                 )
    {
      type::connect_type connect
        ( id::connect (state.next_id())
        , state.id_mapper()
        , parent
        , required ("connect_type", node, "place", state.file_in_progress())
        , required ("connect_type", node, "port", state.file_in_progress())
        );

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "properties")
                {
                  property_map_type (connect.properties(), child, state);
                }
              else if (child_name == "include-properties")
                {
                  const we::type::property::type deeper
                    ( properties_include ( required ( "connect_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         , state
                                         )
                    );

                  util::property::join (state, connect.properties(), deeper);
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "connect_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      return connect;
    }

    // ********************************************************************* //

    static type::place_map_type
    place_map_type ( const xml_node_type * node
                   , state::type & state
                   , const id::transition& parent
                   )
    {
      type::place_map_type place_map
        ( id::place_map (state.next_id())
        , state.id_mapper()
        , parent
        , required ("place_map_type", node, "virtual", state.file_in_progress())
        , required ("place_map_type", node, "real", state.file_in_progress())
        );

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "properties")
                {
                  property_map_type (place_map.prop, child, state);
                }
              else if (child_name == "include-properties")
                {
                  const we::type::property::type deeper
                    ( properties_include ( required ( "connect_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         , state
                                         )
                    );

                  util::property::join (state, place_map.prop, deeper);
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "place_map_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      return place_map;
    }

    // ********************************************************************* //

    id::transition
    transition_type ( const xml_node_type * node
                    , state::type & state
                    , const id::net& parent
                    )
    {
      id::transition id (state.next_id());

      const std::string name
        (required ("transition_type", node, "name", state.file_in_progress()));

      {
        type::transition_type t (id, state.id_mapper(), parent);
      }

      state.id_mapper()->get_ref (id)->path = state.file_in_progress();
      state.id_mapper()->get_ref (id)->name
        (validate_name ( validate_prefix ( name
                                         , "transition"
                                         , state.file_in_progress()
                                         )
                       , "transition"
                       , state.file_in_progress()
                       )
        );
      state.id_mapper()->get_ref (id)
        ->priority = fhg::util::fmap<std::string, petri_net::prio_t>
                   ( boost::lexical_cast<petri_net::prio_t>
                   , optional (node, "priority")
                   );
      state.id_mapper()->get_ref (id)
        ->finline = fhg::util::fmap<std::string, bool>
                  ( fhg::util::read_bool
                  , optional (node, "inline")
                  );
      state.id_mapper()->get_ref (id)
        ->internal = fhg::util::fmap<std::string, bool>
                   ( fhg::util::read_bool
                   , optional (node, "internal")
                   );

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "include-function")
                {
                  const std::string file ( required ( "transition_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         );



                  state.id_mapper()->get_ref (id)
                    ->function_or_use
                    ( id::ref::function
                      ( function_include
                        ( file
                        , state
                        , boost::make_optional
                          (type::function_type::id_parent(id))
                        )
                      , state.id_mapper()
                      )
                    );
                }
              else if (child_name == "use")
                {
                  state.id_mapper()->get_ref (id)
                    ->function_or_use
                    ( type::use_type ( id::use (state.next_id())
                                     , state.id_mapper()
                                     , id
                                     , required ( "transition_type"
                                                , child
                                                , "name"
                                                , state.file_in_progress()
                                                )
                                     )
                    );
                }
              else if (child_name == "defun")
                {
                  state.id_mapper()->get_ref (id)
                    ->function_or_use
                    ( id::ref::function
                      ( function_type
                        ( child
                        , state
                        , boost::make_optional
                          (type::function_type::id_parent(id))
                        )
                      , state.id_mapper()
                      )
                    );
                }
              else if (child_name == "place-map")
                {
                  state.id_mapper()->get_ref (id)
                    ->push_place_map (place_map_type (child, state, id));
                }
              else if (child_name == "connect-in")
                {
                  state.id_mapper()->get_ref (id)
                    ->push_in (connect_type(child, state, id));
                }
              else if (child_name == "connect-out")
                {
                  state.id_mapper()->get_ref (id)
                    ->push_out (connect_type(child, state, id));
                }
              else if (child_name == "connect-inout")
                {
                  state.id_mapper()->get_ref (id)
                    ->push_inout (connect_type (child, state, id));
                }
              else if (child_name == "connect-read")
                {
                  state.id_mapper()->get_ref (id)
                    ->push_read (connect_type(child, state, id));
                }
              else if (child_name == "condition")
                {
                  const type::conditions_type conds
                    ( parse_cdata<type::conditions_type>
                      ( child
                      , state.file_in_progress()
                      )
                    );

                  state.id_mapper()->get_ref (id)
                    ->cond.insert ( state.id_mapper()->get_ref (id)->cond.end()
                                  , conds.begin()
                                  , conds.end()
                                  );
                }
              else if (child_name == "require")
                {
                  require_type ( state.id_mapper()->get_ref (id)->requirements
                               , child
                               , state
                               );
                }
              else if (child_name == "properties")
                {
                  property_map_type ( state.id_mapper()->get_ref (id)->prop
                                    , child
                                    , state
                                    );
                }
              else if (child_name == "include-properties")
                {
                  const we::type::property::type deeper
                    ( properties_include ( required ( "transition_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         , state
                                         )
                    );

                  util::property::join ( state
                                       , state.id_mapper()->get_ref (id)->prop
                                       , deeper
                                       );
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "transition_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      return id;
    }

    // ********************************************************************* //

    id::function
    just_parse (state::type & state, const std::string & input)
    {
      state.set_input (input);

      return state.generic_parse<id::function>
        (boost::bind (parse_function, _1, _2, boost::none), input);
    }

    id::specialize
    specialize_type ( const xml_node_type * node
                    , state::type & state
                    , const id::net& parent
                    )
    {
      id::specialize id (state.next_id());

      {
        type::specialize_type s ( id
                                , state.id_mapper()
                                , parent
                                );
      }

      const boost::filesystem::path& path (state.file_in_progress());

      state.id_mapper()->get_ref(id)->path = path;
      state.id_mapper()->get_ref(id)
        ->name (required ("specialize_type", node, "name", path));
      state.id_mapper()->get_ref(id)
        ->use = required ("specialize_type", node, "use", path);

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "type-map")
                {
                  set_type_map ( child
                               , state
                               , state.id_mapper()->get_ref(id)->type_map
                               );
                }
              else if (child_name == "type-get")
                {
                  set_type_get ( child
                               , state
                               , state.id_mapper()->get_ref(id)->type_get
                               );
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "specialize_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      return id;
    }

    // ********************************************************************* //

    static void
    property_dive ( const xml_node_type * node
                  , state::type & state
                  , we::type::property::type & prop
                  )
    {
      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "property")
                {
                  const std::string key ( required ( "propery_dive"
                                                   , child
                                                   , "key"
                                                   , state.file_in_progress()
                                                   )
                                        );
                  const fhg::util::maybe<std::string>
                    value (optional (child, "value"));

                  typedef std::vector<std::string> cdatas_container_type;

                  const cdatas_container_type cdata
                    ( parse_cdata<cdatas_container_type>
                      ( child
                      , state.file_in_progress()
                      )
                    );

                  state.prop_path().push_back (key);

                  if (cdata.size() > 1)
                    {
                      throw error::property_generic
                        ( "more than one value given"
                        , state.prop_path()
                        , state.file_in_progress()
                        );
                    }

                  if (not value)
                    {
                      if (cdata.empty())
                        {
//                           throw error::property_generic
//                             ( "no value given"
//                             , state.prop_path()
//                             , state.file_in_progress()
//                             );

                          util::property::set_state ( state
                                                    , prop
                                                    , state.prop_path()
                                                    );
                        }
                      else
                        {
                          util::property::set_state ( state
                                                    , prop
                                                    , state.prop_path()
                                                    , cdata.front()
                                                    );
                        }
                    }
                  else
                    {
                      if (!cdata.empty())
                        {
                          throw error::property_generic
                            ( "attribute and content given at the same time"
                            , state.prop_path()
                            , state.file_in_progress()
                            );
                        }

                      util::property::set_state ( state
                                                , prop
                                                , state.prop_path()
                                                , *value
                                                );
                    }

                  state.prop_path().pop_back();
                }
              else if (child_name == "properties")
                {
                  const std::string name ( required ( "property_dive"
                                                    , child
                                                    , "name"
                                                    , state.file_in_progress()
                                                    )
                                         );

                  state.prop_path().push_back (name);

                  property_dive (child, state, prop);

                  state.prop_path().pop_back ();
                }
              else if (child_name == "include-properties")
                {
                  const we::type::property::type deeper
                    ( properties_include ( required ( "property_dive"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         , state
                                         )
                    );

                  util::property::join (state, prop, deeper);
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "property_dive"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }
    }

    int
    property_map_type ( we::type::property::type & prop
                      , const xml_node_type * node
                      , state::type & state
                      )
    {
      if (!state.ignore_properties())
        {
          const std::string name ( required ( "property_map_type"
                                            , node
                                            , "name"
                                            , state.file_in_progress()
                                            )
                                 );

          state.prop_path().push_back (name);

          property_dive (node, state, prop);

          state.prop_path().pop_back ();
        }

      return 0;
    }

    // ********************************************************************* //

    static void token_field_type ( const xml_node_type * node
                                 , state::type & state
                                 , signature::desc_t & tok
                                 )
    {
      const std::string name
        (required ("token_field_type", node, "name", state.file_in_progress()));

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "value")
                {
                  signature::create_literal_field<std::string>
                    (tok, name, child->value(), "token");
                }
              else if (child_name == "field")
                {
                  token_field_type
                    ( child
                    , state
                    , signature::get_or_create_structured_field
                      (tok, name, "token")
                    );
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "token_field_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }
    }

    // ********************************************************************* //

    type::token_type
    token_type ( const xml_node_type * node
               , state::type & state
               , const id::place& parent
               )
    {
      //! \note We can't use a structured_t, as token_field_type takes
      //! a reference to the variant desc_t and also needs that
      //! variant. Also, we have to boost::get the structured_t below,
      //! even though we know it never can be something else.
      signature::desc_t temporary_token ((signature::structured_t()));

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "value")
                {
                  return type::token_type ( id::token (state.next_id())
                                          , state.id_mapper()
                                          , parent
                                          , std::string (child->value())
                                          );
                }
              else if (child_name == "field")
                {
                  token_field_type (child, state, temporary_token);
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "token_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      return type::token_type ( id::token (state.next_id())
                              , state.id_mapper()
                              , parent
                              , boost::get<signature::structured_t>
                                (temporary_token)
                              );
    }

    // ********************************************************************* //

    type::structs_type
    structs_type ( const xml_node_type * node
                 , state::type & state
                 , const id::function& parent
                 )
    {
      //! \note This is only a temporary struct, therefore has no id
      //! or parent.
      type::structs_type v;

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "struct")
                {
                  v.push_back (struct_type (child, state, parent));
                }
              else if (child_name == "include-structs")
                {
                  const type::structs_type structs
                    ( structs_include ( required ( "structs_type"
                                                 , child
                                                 , "href"
                                                 , state.file_in_progress()
                                                 )
                                      , state
                                      , parent
                                      )
                    );

                  v.insert (v.end(), structs.begin(), structs.end());
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "structs_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      return v;
    }

    // ********************************************************************* //

    we::type::property::type
    property_maps_type (const xml_node_type * node, state::type & state)
    {
      we::type::property::type prop;

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "properties")
                {
                  property_map_type (prop, child, state);
                }
              else if (child_name == "include-properties")
                {
                  const we::type::property::type deeper
                    ( properties_include ( required ( "property_maps_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         , state
                                         )
                    );

                  util::property::join (state, prop, deeper);
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "structs_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      return prop;
    }

    // ********************************************************************* //

    id::tmpl tmpl_type ( const xml_node_type * node
                       , state::type & state
                       , const id::net& parent
                       )
    {
      boost::optional<id::ref::function> fun;
      type::tmpl_type::names_type template_parameter;
      fhg::util::maybe<std::string> name (optional (node, "name"));

      const id::tmpl id (state.next_id());

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "template-parameter")
                {
                  const std::string tn (required ( "template-parameter"
                                                 , child
                                                 , "type"
                                                 , state.file_in_progress()
                                                 )
                                       );

                  if (template_parameter.find (tn) != template_parameter.end())
                    {
                      state.warn ( warning::duplicate_template_parameter
                                   ( name
                                   , tn
                                   , state.file_in_progress()
                                   )
                                 );
                    }

                  template_parameter.insert (tn);
                }
              else if (child_name == "defun")
                {
                  fun = id::ref::function
                    ( function_type ( child, state
                                    , boost::make_optional (type::function_type::id_parent(id))
                                    )
                    , state.id_mapper()
                    );
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "template_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      if (not fun)
        {
          throw error::template_without_function
            ( name
            , state.file_in_progress()
            );
        }

      return type::tmpl_type
        ( id
        , state.id_mapper()
        , parent
        , state.file_in_progress()
        , name
        , template_parameter
        , *fun
        ).id();
    }

    // ********************************************************************* //

    id::function
    function_type
      ( const xml_node_type * node
      , state::type & state
      , const boost::optional<type::function_type::id_parent>& parent
      )
    {
      id::expression expression_id (state.next_id());
      id::function id (state.next_id());
      type::expression_type expression ( expression_id
                                       , state.id_mapper()
                                       , id
                                       );
      {
        type::function_type f ( id
                              , state.id_mapper()
                              , expression
                              , parent
                              );
      }

      state.id_mapper()->get_ref (id)->path = state.file_in_progress();
      state.id_mapper()->get_ref (id)->name (optional (node, "name"));
      state.id_mapper()->get_ref (id)->internal =
        fhg::util::fmap<std::string, bool>( fhg::util::read_bool
                                          , optional (node, "internal")
                                          );
      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "in")
                {
                  state.id_mapper()->get_ref (id)
                    ->push_in (port_type (child, state, id));
                }
              else if (child_name == "out")
                {
                  state.id_mapper()->get_ref (id)
                    ->push_out (port_type (child, state, id));
                }
              else if (child_name == "inout")
                {
                  state.id_mapper()->get_ref (id)
                    ->push_inout (port_type (child, state, id));
                }
              else if (child_name == "tunnel")
                {
                  state.id_mapper()->get_ref (id)
                    ->push_tunnel (port_type (child, state, id));
                }
              else if (child_name == "struct")
                {
                  state.id_mapper()->get_ref (id)
                    ->structs.push_back (struct_type (child, state, id));
                }
              else if (child_name == "include-structs")
                {
                  const type::structs_type structs
                    ( structs_include ( required ( "function_type"
                                                 , child
                                                 , "href"
                                                 , state.file_in_progress()
                                                 )
                                      , state
                                      , id
                                      )
                    );

                  state.id_mapper()->get_ref (id)
                    ->structs.insert ( state.id_mapper()->get_ref (id)
                                     ->structs.end()
                                     , structs.begin()
                                     , structs.end()
                                     );
                }
              else if (child_name == "expression")
                {
                  state.id_mapper()->get_ref (id)
                    ->add_expression
                    ( type::expression_type
                      ( id::expression (state.next_id())
                      , state.id_mapper()
                      , id
                      , parse_cdata<type::expressions_type>
                        ( child
                        , state.file_in_progress()
                        )
                      )
                    );
                }
              else if (child_name == "module")
                {
                  state.id_mapper()->get_ref (id)
                    ->f = module_type (child, state, id);
                }
              else if (child_name == "net")
                {
                  state.id_mapper()->get_ref (id)->f = id::ref::net
                    ( ::xml::parse::net_type (child, state, id)
                    , state.id_mapper()
                    );
                }
              else if (child_name == "condition")
                {
                  const type::conditions_type conds
                    ( parse_cdata<type::conditions_type>
                      ( child
                      , state.file_in_progress()
                      )
                    );

                  state.id_mapper()->get_ref (id)
                    ->cond.insert ( state.id_mapper()->get_ref (id)
                                  ->cond.end()
                                  , conds.begin()
                                  , conds.end()
                                  );
                }
              else if (child_name == "properties")
                {
                  property_map_type ( state.id_mapper()->get_ref (id)->prop
                                    , child
                                    , state
                                    );
                }
              else if (child_name == "include-properties")
                {
                  const we::type::property::type deeper
                    ( properties_include ( required ( "function_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         , state
                                         )
                    );

                  util::property::join ( state
                                       , state.id_mapper()->get_ref (id)->prop
                                       , deeper
                                       );
                }
              else if (child_name == "require")
                {
                  require_type ( state.id_mapper()->get_ref (id)->requirements
                               , child
                               , state
                               );
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "function_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      return id;
    }

    // ********************************************************************* //

    type::module_type
    module_type ( const xml_node_type * node
             , state::type & state
             , const id::function& parent
             )
    {
      type::module_type mod
        ( id::module (state.next_id())
        , state.id_mapper()
        , parent
        , required ("module_type", node, "name", state.file_in_progress())
        , required ("module_type", node, "function", state.file_in_progress())
        , state.file_in_progress()
        );

      mod.path = state.file_in_progress();

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "cinclude")
                {
                  const std::string href
                    (required ("module_type", child, "href", state.file_in_progress()));

                  mod.cincludes.push_back (href);
                }
              else if (child_name == "ld")
                {
                  const std::string flag
                    (required ("module_type", child, "flag", state.file_in_progress()));

                  mod.ldflags.push_back (flag);
                }
              else if (child_name == "cxx")
                {
                  const std::string flag
                    (required ("module_type", child, "flag", state.file_in_progress()));

                  mod.cxxflags.push_back (flag);
                }
              else if (child_name == "link")
                {
                  mod.links.push_back ( required ( "module_type"
                                                 , child
                                                 , "href"
                                                 , state.file_in_progress()
                                                 )
                                      );
                }
              else if (child_name == "code")
                {
                  typedef std::vector<std::string> cdatas_container_type;

                  const cdatas_container_type cdata
                    ( parse_cdata<cdatas_container_type>
                      ( child
                      , state.file_in_progress()
                      )
                    );

                  mod.code = fhg::util::join (cdata, "\n");
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "module_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }


      return mod;
    }

    // ********************************************************************* //

    id::net
      net_type ( const xml_node_type * node
               , state::type & state
               , const id::function& parent
               )
    {
      id::net id (state.next_id());

      {
        type::net_type n ( id
                         , state.id_mapper()
                         , parent
                         , state.file_in_progress()
                         );
      }

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "template")
                {
                  state.id_mapper()
                    ->get_ref (id)
                    ->push_template ( id::ref::tmpl
                                      ( tmpl_type (child, state, id)
                                      , state.id_mapper()
                                      )
                                    );
                }
              else if (child_name == "specialize")
                {
                  state.id_mapper()
                    ->get_ref (id)
                    ->push_specialize ( id::ref::specialize
                                        ( specialize_type (child, state, id)
                                        , state.id_mapper()
                                        )
                                      );
                }
              else if (child_name == "place")
                {
                  state.id_mapper()
                    ->get_ref (id)
                    ->push_place ( id::ref::place
                                   ( place_type (child, state, id)
                                   , state.id_mapper()
                                   )
                                 );
                }
              else if (child_name == "transition")
                {
                  state.id_mapper()
                    ->get_ref (id)
                    ->push_transition ( id::ref::transition
                                        ( transition_type (child, state, id)
                                        , state.id_mapper()
                                        )
                                      );
                }
              else if (child_name == "struct")
                {
                  state.id_mapper()->get_ref (parent)
                    ->structs.push_back (struct_type (child, state, parent));
                }
              else if (child_name == "include-structs")
                {
                  std::cerr << "TODO: Deprecate and eliminate net::include-structs.\n";
                  //! \todo deprecate and eliminate
                  const type::structs_type structs
                    ( structs_include ( required ( "net_type"
                                                 , child
                                                 , "href"
                                                 , state.file_in_progress()
                                                 )
                                      , state
                                      , parent
                                      )
                    );

                  state.id_mapper()
                    ->get_ref (id)
                    ->structs.insert ( state.id_mapper()
                                     ->get_ref (id)
                                     ->structs.end()
                                     , structs.begin()
                                     , structs.end()
                                     );
                }
              else if (child_name == "include-template")
                {
                  const std::string file ( required ( "net_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         );
                  const fhg::util::maybe<std::string> as
                    (optional (child, "as"));

                  id::tmpl id_tmpl (template_include (file, state, id));

                  if (as)
                    {
                      if (  state.id_mapper()->get(id_tmpl)->name()
                         && *state.id_mapper()->get(id_tmpl)->name() != *as)
                        {
                          state.warn
                            ( warning::overwrite_template_name_as
                              ( *state.id_mapper()->get(id_tmpl)->name()
                              , *as
                              , state.file_in_progress()
                              )
                            );
                        }

                      state.id_mapper()->get_ref (id_tmpl)->name (*as);
                    }

                  if (not state.id_mapper()->get_ref (id_tmpl)->name())
                    {
                      throw error::top_level_anonymous_template
                        (file, "net_type");
                    }

                  state.id_mapper()->get_ref (id)
                    ->push_template (id::ref::tmpl ( id_tmpl
                                                   , state.id_mapper()
                                                   )
                                    );
                }
              else if (child_name == "properties")
                {
                  property_map_type ( state.id_mapper()
                                    ->get_ref (id)
                                    ->prop
                                    , child
                                    , state
                                    );
                }
              else if (child_name == "include-properties")
                {
                  const we::type::property::type deeper
                    ( properties_include ( required ( "net_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         , state
                                         )
                    );

                  util::property::join ( state
                                       , state.id_mapper()
                                       ->get_ref (id)
                                       ->prop
                                       , deeper
                                       );
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "net_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      return id;
    }

    // ********************************************************************* //

    id::place place_type ( const xml_node_type * node
                         , state::type & state
                         , const id::net& parent
                         )
    {
      id::place id (state.next_id());

      const std::string name
        (required ("place_type", node, "name", state.file_in_progress()));

      {
        type::place_type p
          ( id
          , state.id_mapper()
          , parent
          , validate_name ( validate_prefix ( name
                                            , "place"
                                            , state.file_in_progress()
                                            )
                          , "place"
                          , state.file_in_progress()
                          )
          , required ("place_type", node, "type", state.file_in_progress())
          , fhg::util::fmap<std::string, bool> ( fhg::util::read_bool
                                               , optional (node, "virtual")
                                               )
          );
      }

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "token")
                {
                  state.id_mapper()->get_ref (id)
                    ->push_token (token_type (child, state, id));
                }
              else if (child_name == "properties")
                {
                  property_map_type ( state.id_mapper()->get_ref (id)->prop
                                    , child
                                    , state
                                    );
                }
              else if (child_name == "include-properties")
                {
                  const we::type::property::type deeper
                    ( properties_include ( required ( "place_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         , state
                                         )
                    );

                  util::property::join ( state
                                       , state.id_mapper()->get_ref (id)->prop
                                       , deeper
                                       );
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "place_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      return id;
    }

    // ********************************************************************* //

    type::port_type
    port_type ( const xml_node_type * node
              , state::type & state
              , const id::function& parent
              )
    {
      const std::string name
        (required ("port_type", node, "name", state.file_in_progress()));

      type::port_type port
        ( id::port (state.next_id())
        , state.id_mapper()
        , parent
        , validate_name ( validate_prefix ( name
                                          , "port"
                                          , state.file_in_progress()
                                          )
                        , "port"
                        , state.file_in_progress()
                        )
        , required ("port_type", node, "type", state.file_in_progress())
        , optional (node, "place")
        );

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "properties")
                {
                  property_map_type (port.prop, child, state);
                }
              else if (child_name == "include-properties")
                {
                  const we::type::property::type deeper
                    ( properties_include ( required ( "port_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         , state
                                         )
                    );

                  util::property::join (state, port.prop, deeper);
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "port_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }

      return port;
    }

    // ********************************************************************* //

    void
    struct_field_type ( const xml_node_type * node
                      , state::type & state
                      , signature::desc_t & sig
                      )
    {
      const std::string name
        ( validate_field_name ( required ( "struct_field_type"
                                         , node
                                         , "name"
                                         , state.file_in_progress()
                                         )
                              , state.file_in_progress()
                              )
        );

      const std::string type ( required ( "struct_field_type"
                                        , node
                                        , "type"
                                        , state.file_in_progress()
                                        )
                             );

      if (boost::apply_visitor (signature::visitor::has_field (name), sig))
        {
          throw error::struct_field_redefined (name, state.file_in_progress());
        }

      boost::apply_visitor ( signature::visitor::add_field (name, type)
                           , sig
                           );
    }

    void
    gen_struct_type ( const xml_node_type * node
                    , state::type & state
                    , signature::desc_t & sig
                    )
    {
      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "field")
                {
                  struct_field_type (child, state, sig);
                }
              else if (child_name == "struct")
                {
                  substruct_type (child, state, sig);
                }
              else
                {
                  state.warn
                    ( warning::unexpected_element ( child_name
                                                  , "gen_struct_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }
    }

    void
    substruct_type ( const xml_node_type * node
                   , state::type & state
                   , signature::desc_t & sig
                   )
    {
      const std::string name
        ( validate_field_name ( required ( "substruct_type"
                                         , node
                                         , "name"
                                         , state.file_in_progress()
                                         )
                              , state.file_in_progress()
                              )
        );

      boost::apply_visitor ( signature::visitor::create_structured_field (name)
                           , sig
                           );

      gen_struct_type
        ( node
        , state
        , boost::apply_visitor (signature::visitor::get_field (name), sig)
        );
    }

    type::structure_type
    struct_type ( const xml_node_type * node
                , state::type & state
                , const id::function& parent
                )
    {
      type::structure_type s
        ( id::structure (state.next_id())
        , state.id_mapper()
        , parent
        , validate_field_name ( required ( "struct_type"
                                         , node
                                         , "name"
                                         , state.file_in_progress()
                                         )
                              , state.file_in_progress()
                              )
        , signature::structured_t()
        , state.file_in_progress()
        );

      gen_struct_type (node, state, s.signature());

      return s;
    }
  } // namespace parse
} // namespace xml
