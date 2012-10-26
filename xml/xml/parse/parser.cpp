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

    static type::function_type parse_function ( std::istream & f
                                              , state::type & state
                                              , const type::function_type::id_parent& parent
                                              )
    {
      return generic_parse<type::function_type>
        ( boost::bind (function_type, _1, _2, parent)
        , f
        , state
        , "defun"
        , "parse_function"
        );
    }

    static type::template_type parse_template ( std::istream & f
                                              , state::type & state
                                              , const id::net& parent
                                              )
    {
      return generic_parse<type::template_type>
        ( boost::bind (template_type, _1, _2, parent)
        , f
        , state
        , "template"
        , "parse_template"
        );
    }

    static type::structs_type parse_structs ( std::istream & f
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

    static we::type::property::type
    parse_props (std::istream & f, state::type & state)
    {
      return generic_parse<we::type::property::type>
        (property_maps_type, f, state, "props", "parse_props");
    }

    // ********************************************************************* //

    static type::function_type function_include ( const std::string & file
                                                , state::type & state
                                                , const type::function_type::id_parent& parent
                                                )
    {
      return state.generic_include<type::function_type>
        (boost::bind (parse_function, _1, _2, parent), file);
    }

    static type::template_type template_include ( const std::string & file
                                                , state::type & state
                                                , const id::net& parent
                                                )
    {
      return state.generic_include<type::template_type>
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
        ( required ("connect_type", node, "place", state.file_in_progress())
        , required ("connect_type", node, "port", state.file_in_progress())
        , id::connect (state.next_id())
        , parent
        , state.id_mapper()
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
                  property_map_type (connect.prop, child, state);
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

                  util::property::join (state, connect.prop, deeper);
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
        ( required ("place_map_type", node, "virtual", state.file_in_progress())
        , required ("place_map_type", node, "real", state.file_in_progress())
        , id::place_map (state.next_id())
        , parent
        , state.id_mapper()
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

    type::transition_type
    transition_type ( const xml_node_type * node
                    , state::type & state
                    , const id::net& parent
                    )
    {
      const std::string name
        (required ("transition_type", node, "name", state.file_in_progress()));

      type::transition_type t ( id::transition (state.next_id())
                              , parent
                              , state.id_mapper()
                              );

      t.path = state.file_in_progress();
      t.name (validate_name ( validate_prefix ( name
                                              , "transition"
                                              , state.file_in_progress()
                                              )
                            , "transition"
                            , state.file_in_progress()
                            )
             );
      t.priority = fhg::util::fmap<std::string, petri_net::prio_t>
        ( boost::lexical_cast<petri_net::prio_t>
        , optional (node, "priority")
        );
      t.finline = fhg::util::fmap<std::string, bool>( fhg::util::read_bool
                                                    , optional (node, "inline")
                                                    );
      t.internal =
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
              if (child_name == "include-function")
                {
                  const std::string file ( required ( "transition_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         );

                  t.function_or_use (function_include (file, state, t.id()));
                }
              else if (child_name == "use")
                {
                  t.function_or_use
                    ( type::use_type ( id::use ( state.next_id()

                                               )
                                     , t.id()
                                     , state.id_mapper()
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
                  t.function_or_use (function_type (child, state, t.id()));
                }
              else if (child_name == "place-map")
                {
                  t.push_place_map (place_map_type (child, state, t.id()));
                }
              else if (child_name == "connect-in")
                {
                  t.push_in (connect_type(child, state, t.id()));
                }
              else if (child_name == "connect-out")
                {
                  t.push_out (connect_type(child, state, t.id()));
                }
              else if (child_name == "connect-inout")
                {
                  t.push_inout (connect_type (child, state, t.id()));
                }
              else if (child_name == "connect-read")
                {
                  t.push_read (connect_type(child, state, t.id()));
                }
              else if (child_name == "condition")
                {
                  const type::conditions_type conds
                    ( parse_cdata<type::conditions_type>
                      ( child
                      , state.file_in_progress()
                      )
                    );

                  t.cond.insert (t.cond.end(), conds.begin(), conds.end());
                }
              else if (child_name == "require")
                {
                  require_type (t.requirements, child, state);
                }
              else if (child_name == "properties")
                {
                  property_map_type (t.prop, child, state);
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

                  util::property::join (state, t.prop, deeper);
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

      return t;
    }

    // ********************************************************************* //

    type::function_type
    just_parse (state::type & state, const std::string & input)
    {
      state.set_input (input);

      type::function_type f
        (state.generic_parse<type::function_type>
          (boost::bind (parse_function, _1, _2, boost::blank()), input)
        );

      f.distribute_function (state);

      return f;
    }

    type::specialize_type
    specialize_type ( const xml_node_type * node
                    , state::type & state
                    , const id::net& parent
                    )
    {
      type::specialize_type s ( id::specialize (state.next_id())
                              , parent
                              , state.id_mapper()
                              );

      s.path = state.file_in_progress();
      s.name (required ("specialize_type", node, "name", s.path));
      s.use = required ("specialize_type", node, "use", s.path);

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
                  set_type_map (child, state, s.type_map);
                }
              else if (child_name == "type-get")
                {
                  set_type_get (child, state, s.type_get);
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

      return s;
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
                                          , parent
                                          , state.id_mapper()
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
                              , parent
                              , state.id_mapper()
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

    type::template_type template_type ( const xml_node_type * node
                                      , state::type & state
                                      , const id::net& parent
                                      )
    {
      boost::optional<type::function_type> fun;
      type::template_type::names_type template_parameter;
      fhg::util::maybe<std::string> name (optional (node, "name"));

      const id::tmpl template_id (state.next_id());

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
                  fun = function_type (child, state, template_id);
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

      return type::template_type
        ( template_id
        , parent
        , state.id_mapper()
        , state.file_in_progress()
        , name
        , template_parameter
        , *fun
        );
    }

    // ********************************************************************* //

    type::function_type function_type ( const xml_node_type * node
                                      , state::type & state
                                      , const type::function_type::id_parent& parent
                                      )
    {
      id::expression expression_id (state.next_id());
      id::function function_id (state.next_id());
      type::expression_type expression ( expression_id
                                       , function_id
                                       , state.id_mapper()
                                       );
      type::function_type f ( expression
                            , function_id
                            , parent
                            , state.id_mapper()
                            );

      f.path = state.file_in_progress();
      f.name (optional (node, "name"));
      f.internal =
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
                  f.push_in (port_type (child, state, f.id()));
                }
              else if (child_name == "out")
                {
                  f.push_out (port_type (child, state, f.id()));
                }
              else if (child_name == "inout")
                {
                  f.push_inout (port_type (child, state, f.id()));
                }
              else if (child_name == "tunnel")
                {
                  f.push_tunnel (port_type (child, state, f.id()));
                }
              else if (child_name == "struct")
                {
                  f.structs.push_back (struct_type (child, state, f.id()));
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
                                      , f.id()
                                      )
                    );

                  f.structs.insert ( f.structs.end()
                                   , structs.begin()
                                   , structs.end()
                                   );
                }
              else if (child_name == "expression")
                {
                  f.add_expression
                    ( type::expression_type
                      ( parse_cdata<type::expressions_type>
                        ( child
                        , state.file_in_progress()
                        )
                      , id::expression (state.next_id())
                      , f.id()
                      , state.id_mapper()
                      )
                    );
                }
              else if (child_name == "module")
                {
                  f.f = mod_type (child, state, f.id());
                }
              else if (child_name == "net")
                {
                  f.f = ::xml::parse::net_type (child, state, f.id(), f);
                }
              else if (child_name == "condition")
                {
                  const type::conditions_type conds
                    ( parse_cdata<type::conditions_type>
                      ( child
                      , state.file_in_progress()
                      )
                    );

                  f.cond.insert (f.cond.end(), conds.begin(), conds.end());
                }
              else if (child_name == "properties")
                {
                  property_map_type (f.prop, child, state);
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

                  util::property::join (state, f.prop, deeper);
                }
              else if (child_name == "require")
                {
                  require_type (f.requirements, child, state);
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

      return f;
    }

    // ********************************************************************* //

    type::mod_type
    mod_type ( const xml_node_type * node
             , state::type & state
             , const id::function& parent
             )
    {
      type::mod_type mod
        ( id::module (state.next_id())
        , parent
        , state.id_mapper()
        , required ("mod_type", node, "name", state.file_in_progress())
        , required ("mod_type", node, "function", state.file_in_progress())
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
                    (required ("mod_type", child, "href", state.file_in_progress()));

                  mod.cincludes.push_back (href);
                }
              else if (child_name == "ld")
                {
                  const std::string flag
                    (required ("mod_type", child, "flag", state.file_in_progress()));

                  mod.ldflags.push_back (flag);
                }
              else if (child_name == "cxx")
                {
                  const std::string flag
                    (required ("mod_type", child, "flag", state.file_in_progress()));

                  mod.cxxflags.push_back (flag);
                }
              else if (child_name == "link")
                {
                  mod.links.push_back ( required ( "mod_type"
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
                                                  , "mod_type"
                                                  , state.file_in_progress()
                                                  )
                    );
                }
            }
        }


      return mod;
    }

    // ********************************************************************* //

    type::net_type
      net_type ( const xml_node_type * node
               , state::type & state
               , const id::function& parent
               , type::function_type& parent_fun
               )
    {
      type::net_type n ( id::net (state.next_id())
                       , parent
                       , state.id_mapper()
                       );

      n.path = state.file_in_progress();

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
                  n.push_template (template_type (child, state, n.id()));
                }
              else if (child_name == "specialize")
                {
                  n.push_specialize (specialize_type (child, state, n.id()), state);
                }
              else if (child_name == "place")
                {
                  n.push_place (place_type (child, state, n.id()));
                }
              else if (child_name == "transition")
                {
                  n.push_transition (transition_type (child, state, n.id()));
                }
              else if (child_name == "struct")
                {
                  parent_fun.structs.push_back
                    (struct_type (child, state, n.parent()));
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
                                      , n.parent()
                                      )
                    );

                  n.structs.insert ( n.structs.end()
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

                  type::template_type tmpl
                    (template_include (file, state, n.id()));

                  if (as)
                    {
                      if (tmpl.name() && *tmpl.name() != *as)
                        {
                          state.warn
                            ( warning::overwrite_template_name_as
                              ( *tmpl.name()
                              , *as
                              , state.file_in_progress()
                              )
                            );
                        }

                      tmpl.name (*as);
                    }

                  if (not tmpl.name())
                    {
                      throw error::top_level_anonymous_template
                        (file, "net_type");
                    }

                  n.push_template (tmpl);
                }
              else if (child_name == "properties")
                {
                  property_map_type (n.prop, child, state);
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

                  util::property::join (state, n.prop, deeper);
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

      return n;
    }

    // ********************************************************************* //

    type::place_type place_type ( const xml_node_type * node
                                       , state::type & state
                                       , const id::net& parent
                                       )
    {
      const std::string name
        (required ("place_type", node, "name", state.file_in_progress()));

      type::place_type p
        ( validate_name ( validate_prefix ( name
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
        , id::place (state.next_id())
        , parent
        , state.id_mapper()
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
              if (child_name == "token")
                {
                  p.push_token (token_type (child, state, p.id()));
                }
              else if (child_name == "properties")
                {
                  property_map_type (p.prop, child, state);
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

                  util::property::join (state, p.prop, deeper);
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

      return p;
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
        ( validate_name ( validate_prefix ( name
                                          , "port"
                                          , state.file_in_progress()
                                          )
                        , "port"
                        , state.file_in_progress()
                        )
        , required ("port_type", node, "type", state.file_in_progress())
        , optional (node, "place")
        , id::port (state.next_id())
        , parent
        , state.id_mapper()
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

    type::struct_t
    struct_type ( const xml_node_type * node
                , state::type & state
                , const id::function& parent
                )
    {
      type::struct_t s ( id::structure (state.next_id())
                       , parent
                       , state.id_mapper()
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
