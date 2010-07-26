// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_PARSER_HPP
#define _XML_PARSE_PARSER_HPP

#include <xml/parse/rapidxml/1.13/rapidxml.hpp>
#include <xml/parse/rapidxml/1.13/rapidxml_utils.hpp>

#include <xml/parse/util.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/warning.hpp>
#include <xml/parse/types.hpp>
#include <xml/parse/state.hpp>

#include <we/type/signature.hpp>
#include <we/type/id.hpp>
#include <we/type/property.hpp>
#include <we/util/read.hpp>

#include <boost/lexical_cast.hpp>

// ************************************************************************* //

namespace xml
{
  namespace parse
  {
    // ********************************************************************* //

    static type::connect_type connect_type ( const xml_node_type *
                                           , state::type &
                                           );
    static type::function_type function_type ( const xml_node_type *
                                             , state::type &
                                             );
    static type::mod_type mod_type (const xml_node_type *, state::type &);
    static type::net_type net_type (const xml_node_type *, state::type &);
    static type::place_type place_type (const xml_node_type *, state::type &);
    static type::port_type port_type (const xml_node_type *, state::type &);
    static void gen_struct_type ( const xml_node_type *, state::type &
                                , signature::desc_t &
                                );
    static void substruct_type ( const xml_node_type *, state::type &
                               , signature::desc_t &
                               );
    static type::struct_t struct_type (const xml_node_type *, state::type &);
    static type::token_type token_type (const xml_node_type *, state::type &);
    static type::transition_type transition_type ( const xml_node_type *
                                                 , state::type &
                                                 );
    static type::specialize_type specialize_type ( const xml_node_type *
                                                 , state::type &
                                                 );

    static int property_map_type ( we::type::property::type &
                                 , const xml_node_type *
                                 , state::type &
                                 );
    static we::type::property::type
    property_map_list_type (const xml_node_type *, state::type &);

    static type::struct_vec_type structs_type ( const xml_node_type *
                                              , state::type & state
                                              );

    static type::function_type parse_function (std::istream &, state::type &);
    static type::function_type parse_template (std::istream &, state::type &);
    static type::struct_vec_type parse_structs (std::istream &, state::type &);
    static we::type::property::type parse_props (std::istream &, state::type &);

    // ********************************************************************* //

    static type::function_type
    function_include (const std::string & file, state::type & state)
    {
      return state.generic_include<type::function_type> (parse_function, file);
    }

    static type::function_type
    template_include (const std::string & file, state::type & state)
    {
      return state.generic_include<type::function_type> (parse_template, file);
    }

    static type::struct_vec_type
    structs_include (const std::string & file, state::type & state)
    {
      return state.generic_include<type::struct_vec_type> (parse_structs, file);
    }

    static we::type::property::type
    properties_include (const std::string & file, state::type & state)
    {
      return
        state.generic_include<we::type::property::type> (parse_props, file);
    }

    // ********************************************************************* //

    template<typename T>
    static T
    generic_parse ( T (*parse)(const xml_node_type *, state::type &)
                  , std::istream & f
                  , state::type & state
                  , const std::string & name_wanted
                  , const std::string & pre
                  )
    {
      xml_document_type doc;

      input_type inp (f);

      doc.parse < rapidxml::parse_full
                | rapidxml::parse_trim_whitespace
                | rapidxml::parse_normalize_whitespace
                > (inp.data())
        ;

      xml_node_type * node (doc.first_node());

      if (!node)
        {
          throw error::no_elements_given (pre, state.file_in_progress());
        }

      skip (node, rapidxml::node_declaration);

      const std::string name (name_element (node, state.file_in_progress()));

      if (!node)
        {
          throw error::no_elements_given (pre, state.file_in_progress());
        }

      if (name != name_wanted)
        {
          state.warn
            (warning::unexpected_element (name, pre, state.file_in_progress()));
        }

      if (node->next_sibling())
        {
          throw error::more_than_one_definition (pre, state.file_in_progress());
        }

      return parse (node, state);
    };

    static type::function_type
    parse_function (std::istream & f, state::type & state)
    {
      return generic_parse (function_type, f, state, "defun", "parse_function");
    }

    static type::function_type
    parse_template (std::istream & f, state::type & state)
    {
      return
        generic_parse (function_type, f, state, "template", "parse_template");
    }

    static type::struct_vec_type
    parse_structs (std::istream & f, state::type & state)
    {
      return generic_parse (structs_type, f, state, "structs", "parse_structs");
    }

    static we::type::property::type
    parse_props (std::istream & f, state::type & state)
    {
      return generic_parse ( property_map_list_type
                           , f
                           , state
                           , "props"
                           , "parse_props"
                           );
    }

    // ********************************************************************* //

    static type::struct_vec_type
    structs_type (const xml_node_type * node, state::type & state)
    {
      type::struct_vec_type v;

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
                  v.push_back (struct_type (child, state));
                }
              else if (child_name == "include-structs")
                {
                  const type::struct_vec_type structs 
                    ( structs_include ( required ( "structs_type"
                                                 , child
                                                 , "href"
                                                 , state.file_in_progress()
                                                 )
                                      , state
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

    static we::type::property::type
    property_map_list_type (const xml_node_type * node, state::type & state)
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
                    ( properties_include ( required ( "property_map_list_type"
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

    static type::connect_type
    connect_type (const xml_node_type * node, state::type & state)
    {
      type::connect_type connect
        ( required ("connect_type", node, "place", state.file_in_progress())
        , required ("connect_type", node, "port", state.file_in_progress())
        , state.level()
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

    static type::function_type
    function_type (const xml_node_type * node, state::type & state)
    {
      type::function_type f;

      f.path = state.file_in_progress();
      f.level = state.level();
      f.name = optional (node, "name");
      f.internal = fmap<std::string, bool>( read_bool
                                          , optional (node, "internal")
                                          );
      f.was_template = false;

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
                  f.push_in (port_type (child, state));
                }
              else if (child_name == "out")
                {
                  f.push_out (port_type (child, state));
                }
              else if (child_name == "struct")
                {
                  f.structs.push_back (struct_type (child, state));
                }
              else if (child_name == "include-structs")
                {
                  const type::struct_vec_type structs 
                    ( structs_include ( required ( "function_type"
                                                 , child
                                                 , "href"
                                                 , state.file_in_progress()
                                                 )
                                      , state
                                      )
                    );

                  f.structs.insert ( f.structs.end()
                                   , structs.begin()
                                   , structs.end()
                                   );
                }
              else if (child_name == "expression")
                {
                  f.f = type::expression_type
                    (parse_cdata (child, state.file_in_progress()));
                }
              else if (child_name == "module")
                {
                  f.f = mod_type (child, state);
                }
              else if (child_name == "net")
                {
                  ++state.level();
                  
                  f.f = net_type (child, state);

                  --state.level();
                }
              else if (child_name == "condition")
                {
                  const type::cond_vec_type conds
                    (parse_cdata (child, state.file_in_progress()));

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

    static type::mod_type
    mod_type (const xml_node_type * node, state::type & state)
    {
      return type::mod_type
        ( required ("mod_type", node, "name", state.file_in_progress())
        , required ("mod_type", node, "function", state.file_in_progress())
        );
    }

    // ********************************************************************* //

    static type::net_type
    net_type (const xml_node_type * node, state::type & state)
    {
      type::net_type n;

      n.path = state.file_in_progress();
      n.level = state.level();

      ++state.level();

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child ? child->next_sibling() : child
          )
        {
          const std::string child_name
            (name_element (child, state.file_in_progress()));

          if (child)
            {
              if (child_name == "defun")
                {
                  n.push_function (function_type (child, state));
                }
              else if (child_name == "template")
                {
                  n.push_template (function_type (child, state));
                }
              else if (child_name == "specialize")
                {
                  n.push_specialize (specialize_type (child, state), state);
                }
              else if (child_name == "place")
                {
                  n.push_place (place_type (child, state));
                }
              else if (child_name == "transition")
                {
                  n.push_transition (transition_type (child, state));
                }
              else if (child_name == "struct")
                {
                  n.structs.push_back (struct_type (child, state));
                }
              else if (child_name == "include-structs")
                {
                  const type::struct_vec_type structs 
                    ( structs_include ( required ( "net_type"
                                                 , child
                                                 , "href"
                                                 , state.file_in_progress()
                                                 )
                                      , state
                                      )
                    );

                  n.structs.insert ( n.structs.end()
                                   , structs.begin()
                                   , structs.end()
                                   );
                }
              else if (child_name == "include-function")
                {
                  const std::string file ( required ( "net_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         );
                  const maybe<std::string> as (optional (child, "as"));

                  type::function_type fun (function_include (file, state));

                  if (as.isJust())
                    {
                      if (fun.name.isJust() && *fun.name != *as)
                        {
                          state.warn 
                            ( warning::overwrite_function_name_as
                              ( *fun.name
                              , *as
                              , state.file_in_progress()
                              )
                            );
                        }

                      fun.name = *as;
                    }

                  if (fun.name.isNothing())
                    {
                      throw error::top_level_anonymous_function
                        (file, "net_type");
                    }

                  n.push_function (fun);
                }
              else if (child_name == "include-template")
                {
                  const std::string file ( required ( "net_type"
                                                    , child
                                                    , "href"
                                                    , state.file_in_progress()
                                                    )
                                         );
                  const maybe<std::string> as (optional (child, "as"));

                  type::function_type tmpl (template_include (file, state));

                  if (as.isJust())
                    {
                      if (tmpl.name.isJust() && *tmpl.name != *as)
                        {
                          state.warn 
                            ( warning::overwrite_template_name_as
                              ( *tmpl.name
                              , *as
                              , state.file_in_progress()
                              )
                            );
                        }

                      tmpl.name = *as;
                    }

                  if (tmpl.name.isNothing())
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

      --state.level();

      return n;
    }

    // ********************************************************************* //

    static type::place_type
    place_type (const xml_node_type * node, state::type & state)
    {
      const std::string name
        (required ("place_type", node, "name", state.file_in_progress()));

      type::place_type p
        ( validate_prefix (name, "place", state.file_in_progress())
        , required ("place_type", node, "type", state.file_in_progress())
        , fmap<std::string, petri_net::capacity_t>
          ( &::we::util::reader<petri_net::capacity_t>::read
          , optional (node, "capacity")
          )
        );

      p.level = state.level();

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
                  p.push_token (token_type (child, state));
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

    static type::port_type
    port_type (const xml_node_type * node, state::type & state)
    {
      type::port_type port
        ( required ("port_type", node, "name", state.file_in_progress())
        , required ("port_type", node, "type", state.file_in_progress())
        , optional (node, "place")
        , state.level() + 2
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
                  const maybe<std::string> value (optional (child, "value"));
                  const std::vector<std::string> cdata
                    (parse_cdata (child, state.file_in_progress()));

                  state.prop_path().push_back (key);

                  if (cdata.size() > 1)
                    {
                      throw error::property_generic 
                        ( "more than one value given"
                        , state.prop_path()
                        , state.file_in_progress()
                        );
                    }

                  if (value.isNothing())
                    {
                      if (cdata.empty())
                        {
                          throw error::property_generic
                            ( "no value given"
                            , state.prop_path()
                            , state.file_in_progress()
                            );
                        }

                      util::property::set_state ( state
                                                , prop
                                                , state.prop_path()
                                                , cdata.front()
                                                );
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

    static int
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

    static void
    struct_field_type ( const xml_node_type * node
                      , state::type & state
                      , signature::desc_t & sig
                      )
    {
      const std::string name ( required ( "struct_field_type"
                                        , node
                                        , "name"
                                        , state.file_in_progress()
                                        )
                             );
      const std::string type ( required ( "struct_field_type"
                                        , node
                                        , "type"
                                        , state.file_in_progress()
                                        )
                             );

      boost::apply_visitor ( signature::visitor::add_field (name, type)
                           , sig
                           );
    }

    static void
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

    static void
    substruct_type ( const xml_node_type * node
                   , state::type & state
                   , signature::desc_t & sig
                   )
    {
      const std::string name
        (required ("substruct_type", node, "name", state.file_in_progress()));

      boost::apply_visitor ( signature::visitor::create_structured_field (name)
                           , sig
                           );

      gen_struct_type 
        ( node
        , state
        , boost::apply_visitor (signature::visitor::get_field (name), sig)
        );
    }

    static type::struct_t
    struct_type (const xml_node_type * node, state::type & state)
    {
      type::struct_t s;

      s.path = state.file_in_progress();
      s.name = required ("struct_type", node, "name", state.file_in_progress());
      s.sig = signature::structured_t();
      s.level = state.level();

      gen_struct_type (node, state, s.sig);

      return s;
    }

    // ********************************************************************* //

    static void
    token_field_type ( const xml_node_type * node
                     , state::type & state
                     , type::token_type & tok
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
                  boost::apply_visitor
                    ( signature::visitor::create_literal_field<std::string> 
                      ( name
                      , std::string (child->value())
                      , "token"
                      )
                    , tok
                    );
                }
              else if (child_name == "field")
                {
                  token_field_type 
                    ( child
                    , state
                    , boost::apply_visitor 
                      ( signature::visitor::get_or_create_structured_field
                        (name, "token")
                      , tok
                      )
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

    static type::token_type
    token_type (const xml_node_type * node, state::type & state)
    {
      type::token_type tok = signature::structured_t();

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
                  return type::token_type (std::string (child->value()));
                }
              else if (child_name == "field")
                {
                  token_field_type (child, state, tok);
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

      return tok;
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

    static type::specialize_type
    specialize_type (const xml_node_type * node, state::type & state)
    {
      type::specialize_type s;

      s.name =
        required ("specialize_type", node, "name", state.file_in_progress());
      s.use =
        required ("specialize_type", node, "use", state.file_in_progress());
      s.level = state.level();

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

    static type::transition_type
    transition_type (const xml_node_type * node, state::type & state)
    {
      const std::string name
        (required ("transition_type", node, "name", state.file_in_progress()));

      type::transition_type t;

      t.path = state.file_in_progress();
      t.level = state.level();
      t.name = validate_prefix (name, "transition", state.file_in_progress());
      t.priority = fmap<std::string, petri_net::prio_t>
        ( boost::lexical_cast<petri_net::prio_t>
        , optional (node, "priority")
        );
      t.finline = fmap<std::string, bool>( read_bool
                                         , optional (node, "inline")
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

                  state.level() += 2;;

                  t.f = function_include (file, state);

                  state.level() -= 2;
                }
              else if (child_name == "use")
                {
                  t.f = type::use_type ( required ( "transition_type"
                                                  , child
                                                  , "name"
                                                  , state.file_in_progress()
                                                  )
                                       , optional (child, "as")
                                       , state.level() + 2
                                       );
                }
              else if (child_name == "defun")
                {
                  state.level() += 2;

                  t.f = function_type (child, state);

                  state.level() -= 2;
                }
              else if (child_name == "connect-in")
                {
                  t.push_in (connect_type(child, state));
                }
              else if (child_name == "connect-out")
                {
                  t.push_out (connect_type(child, state));
                }
              else if (child_name == "connect-read")        
                {
                  t.push_read (connect_type(child, state));
                }
              else if (child_name == "condition")
                {
                  const type::cond_vec_type conds
                    (parse_cdata (child, state.file_in_progress()));

                  t.cond.insert (t.cond.end(), conds.begin(), conds.end());
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

    static type::function_type
    frontend (state::type & state, const std::string & input)
    {
      type::function_type f
        ( (input == "-") 
        ? parse_function (std::cin, state)
        : function_include (input, state)
        );

      f.specialize (state);
      f.resolve (state, f.forbidden_below());
      f.type_check (state);

      return f;
    }
  }
}

#endif
