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

#include <xml/parse/headerlist.hpp>
#include <xml/parse/headergen.hpp>

#include <xml/parse/util/mk_fstream.hpp>

#include <we/type/signature.hpp>
#include <we/type/id.hpp>
#include <we/type/property.hpp>

#include <fhg/util/read.hpp>
#include <fhg/util/maybe.hpp>
#include <fhg/util/read_bool.hpp>
#include <fhg/util/join.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include <sstream>
#include <vector>

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
    property_maps_type (const xml_node_type *, state::type &);

    static type::structs_type structs_type ( const xml_node_type *
                                              , state::type & state
                                              );

    static type::function_type parse_function (std::istream &, state::type &);
    static type::function_type parse_template (std::istream &, state::type &);
    static type::structs_type parse_structs (std::istream &, state::type &);
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

    static type::structs_type
    structs_include (const std::string & file, state::type & state)
    {
      return state.generic_include<type::structs_type> (parse_structs, file);
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

      try
        {
          doc.parse < rapidxml::parse_full
                    | rapidxml::parse_trim_whitespace
                    | rapidxml::parse_normalize_whitespace
                    > (inp.data())
                    ;
        }
      catch (const rapidxml::parse_error & e)
        {
          int line = 1;
          int col = 0;

          for ( char * pos = const_cast<char *>(inp.data())
              ; pos != e.where<char>()
              ; ++pos
              )
            {
              col += 1;

              if (*pos == '\n')
                {
                  col = 0;
                  line += 1;
                }
            }

          std::ostringstream oss;

          oss << "Parse error [" << line << ":" << col << "]: " << e.what();

          throw rapidxml::parse_error (oss.str().c_str(), e.where<void>());
        }

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

      xml_node_type * sib (node->next_sibling());

      skip (sib, rapidxml::node_comment);

      if (sib)
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

    static type::structs_type
    parse_structs (std::istream & f, state::type & state)
    {
      return generic_parse (structs_type, f, state, "structs", "parse_structs");
    }

    static we::type::property::type
    parse_props (std::istream & f, state::type & state)
    {
      return generic_parse ( property_maps_type
                           , f
                           , state
                           , "props"
                           , "parse_props"
                           );
    }

    // ********************************************************************* //

    static type::structs_type
    structs_type (const xml_node_type * node, state::type & state)
    {
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
                  v.push_back (struct_type (child, state));
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

    static type::connect_type
    connect_type (const xml_node_type * node, state::type & state)
    {
      type::connect_type connect
        ( required ("connect_type", node, "place", state.file_in_progress())
        , required ("connect_type", node, "port", state.file_in_progress())
        , state.next_id()
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
    place_map_type (const xml_node_type * node, state::type & state)
    {
      type::place_map_type place_map
        ( required ("place_map_type", node, "virtual", state.file_in_progress())
        , required ("place_map_type", node, "real", state.file_in_progress())
        , state.next_id()
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
      const bool mandatory (mmandatory.isJust() ? *mmandatory : true);

      requirements.set (key, mandatory);

      // collect all the requirements for the top level function
      state.set_requirement (key, mandatory);
    }

    // ********************************************************************* //

    static type::function_type
    function_type (const xml_node_type * node, state::type & state)
    {
      type::function_type f (state.next_id());

      f.path = state.file_in_progress();
      f.name = optional (node, "name");
      f.internal =
        fhg::util::fmap<std::string, bool>( fhg::util::read_bool
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
              else if (child_name == "inout")
                {
                  f.push_inout (port_type (child, state));
                }
              else if (child_name == "tunnel")
                {
                  f.push_tunnel (port_type (child, state));
                }
              else if (child_name == "struct")
                {
                  f.structs.push_back (struct_type (child, state));
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
                      )
                    );
                }
              else if (child_name == "module")
                {
                  f.f = mod_type (child, state);
                }
              else if (child_name == "net")
                {
                  f.f = net_type (child, state);
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
              else if (child_name == "template-parameter")
                {
                  if (std::string (node->name()) != "template")
                    {
                      state.warn ( warning::ignore_template_parameter
                                   ( f.name
                                   , state.file_in_progress()
                                   )
                                 );
                    }
                  else
                    {
                      const std::string tn (required ( "template-parameter"
                                                     , child
                                                     , "type"
                                                     , state.file_in_progress()
                                                     )
                                           );

                      if (f.typenames().find (tn) != f.typenames().end())
                        {
                          state.warn ( warning::duplicate_template_parameter
                                       ( f.name
                                       , tn
                                       , state.file_in_progress()
                                       )
                                     );
                        }

                      f.insert_typename (tn);
                    }
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
    mod_type ( const xml_node_type * node, state::type & state)
    {
      type::mod_type mod
        ( required ("mod_type", node, "name", state.file_in_progress())
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

    static type::net_type
    net_type (const xml_node_type * node, state::type & state)
    {
      type::net_type n (state.next_id());

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
              if (child_name == "defun")
                {
                  n.push_function (function_type (child, state));

                  state.deprecated
                    ("Free flowing function in subnetwork."
                    " Use seperate file and <include-function> instead."
                    );
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
                  const type::structs_type structs
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
                  const fhg::util::maybe<std::string> as
                    (optional (child, "as"));

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
                  const fhg::util::maybe<std::string> as
                    (optional (child, "as"));

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

      return n;
    }

    // ********************************************************************* //

    static type::place_type
    place_type (const xml_node_type * node, state::type & state)
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
        , state.next_id()
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
        , state.next_id()
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

                  if (value.isNothing())
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

    static type::struct_t
    struct_type (const xml_node_type * node, state::type & state)
    {
      type::struct_t s;

      s.path = state.file_in_progress();
      s.name = validate_field_name ( required ( "struct_type"
                                              , node
                                              , "name"
                                              , state.file_in_progress()
                                              )
                                   , state.file_in_progress()
                                   );
      s.sig = signature::structured_t();

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
      type::specialize_type s (state.next_id());

      s.path = state.file_in_progress();
      s.name = required ("specialize_type", node, "name", s.path);
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

    static type::transition_type
    transition_type (const xml_node_type * node, state::type & state)
    {
      const std::string name
        (required ("transition_type", node, "name", state.file_in_progress()));

      type::transition_type t (state.next_id());

      t.path = state.file_in_progress();
      t.name = validate_name ( validate_prefix ( name
                                               , "transition"
                                               , state.file_in_progress()
                                               )
                             , "transition"
                             , state.file_in_progress()
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

                  t.function_or_use (function_include (file, state));
                }
              else if (child_name == "use")
                {
                  t.function_or_use
                    ( type::use_type ( required ( "transition_type"
                                                , child
                                                , "name"
                                                , state.file_in_progress()
                                                )
                                     )
                    );
                }
              else if (child_name == "defun")
                {
                  t.function_or_use (function_type (child, state));
                }
              else if (child_name == "place-map")
                {
                  t.push_place_map (place_map_type (child, state));
                }
              else if (child_name == "connect-in")
                {
                  t.push_in (connect_type(child, state));
                }
              else if (child_name == "connect-out")
                {
                  t.push_out (connect_type(child, state));
                }
              else if (child_name == "connect-inout")
                {
                  t.push_inout (connect_type (child, state));
                }
              else if (child_name == "connect-read")
                {
                  t.push_read (connect_type(child, state));
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

    namespace dependencies
    {
      template<typename Stream>
      class wrapping_word_stream
      {
      private:
        const std::size_t _max_len;
        mutable std::size_t _len;
        Stream& _stream;
      public:
        wrapping_word_stream (Stream& stream, const std::size_t max = 75)
          : _max_len (max)
          , _len (0)
          , _stream (stream)
        {}

        void put (const std::string& w) const
        {
          if (_len + w.size() > _max_len)
            {
              _stream << " \\"; newl(); append (" ");
            }
          else if (_len > 0)
            {
              append (" ");
            }

          append (w);
        }

        void append (const std::string& s) const
        {
          _stream << s;

          _len += s.size();
        }

        void newl () const
        {
          _stream << std::endl;

          _len = 0;
        }
      };

      class quote
      {
      private:
        std::string _quoted;

      public:
        quote (const std::string& s) : _quoted ()
        {
          std::string::const_iterator pos (s.begin());
          const std::string::const_iterator end (s.end());

          while (pos != end)
            {
              switch (*pos)
                {
                case ' ': _quoted += "\\ "; break;
                case '$': _quoted += "$$"; break;
                default: _quoted += *pos; break;
                }

              ++pos;
            }
        };

        operator const std::string& () const { return _quoted; }
      };


      inline std::string quote_for_list (const std::string& s)
      {
        std::string quoted;
        std::string::const_iterator pos (s.begin());
        const std::string::const_iterator end (s.end());

        while (pos != end)
          {
            switch (*pos)
              {
              case ' ': quoted += "\\ "; break;
              default: quoted += *pos; break;
              }

            ++pos;
          }

        return quoted;
      }

      inline std::string quote_for_list (const boost::filesystem::path& p)
      {
        return quote_for_list (p.string());
      }

      template<typename Stream>
      void mk ( const state::type& state
              , const std::string& input
              , Stream& stream
              )
      {
        wrapping_word_stream<Stream> wrapping_stream (stream);

        if (state.dependencies_target().size() > 0)
          {
            BOOST_FOREACH ( const std::string& target
                          , state.dependencies_target()
                          )
              {
                wrapping_stream.put (target);
              }
          }

        if (state.dependencies_target_quoted().size() > 0)
          {
            BOOST_FOREACH ( const std::string& target
                          , state.dependencies_target_quoted()
                          )
              {
                wrapping_stream.put (quote (target));
              }
          }

        if (  (state.dependencies_target().size() == 0)
           && (state.dependencies_target_quoted().size() == 0)
           )
          {
            wrapping_stream.put (input);
          }

        wrapping_stream.append (":");

        BOOST_FOREACH ( const boost::filesystem::path& path
                      , state.dependencies()
                      )
          {
            const std::string& dep (path.string());

            if (dep != input)
              {
                wrapping_stream.put (dep);
              }
          }

        wrapping_stream.newl();

        if (state.dependencies_add_phony_targets())
          {
            BOOST_FOREACH ( const boost::filesystem::path& path
                          , state.dependencies()
                          )
              {
                const std::string& dep (path.string());

                if (dep != input)
                  {
                    wrapping_stream.newl();
                    wrapping_stream.append (dep);
                    wrapping_stream.append(":");
                    wrapping_stream.newl();
                  }
              }
          }
      }
    }

    // ********************************************************************* //

    inline type::function_type
    just_parse (state::type & state, const std::string & input)
    {
      state.set_input (input);

      type::function_type f
        (state.generic_parse<type::function_type> (parse_function, input));

      f.distribute_function (state);

      return f;
    }

    inline type::function_type
    frontend (state::type & state, const std::string & input)
    {
      type::function_type f (just_parse (state, input));

      if (state.dump_xml_file().size() > 0)
        {
          const std::string& file (state.dump_xml_file());
          std::ofstream stream (file.c_str());

          if (!stream.good())
            {
              throw error::could_not_open_file (file);
            }

          fhg::util::xml::xmlstream s (stream);

          xml::parse::type::dump::dump (s, f, state);
        }

      // set all the collected requirements to the top level function
      f.requirements = state.requirements();

      f.specialize (state);
      f.resolve (state, f.forbidden_below());
      f.type_check (state);
      f.sanity_check (state);

      if (state.path_to_cpp().size() > 0)
        {
          type::fun_info_map m;

          type::find_module_calls (state, f, m);

          type::mk_wrapper (state, m);
          type::mk_makefile (state, m);

          includes::descrs_type descrs;

          includes::mks (descrs);
          includes::we_header_gen (state, descrs);

          type::struct_to_cpp (state, f);
        }

      if (state.dump_dependenciesD())
        {
          state.dump_dependencies() = input + ".d";
        }

      if (state.dump_dependencies().size() > 0)
        {
          const std::string& file (state.dump_dependencies());
          std::ofstream stream (file.c_str());

          if (!stream.good())
            {
              throw error::could_not_open_file (file);
            }

          dependencies::mk (state, input, stream);
        }

      if (state.list_dependencies().size() > 0)
        {
          const std::string& file (state.list_dependencies());
          std::ofstream stream (file.c_str());

          if (not stream)
            {
              throw error::could_not_open_file (file);
            }

          stream << dependencies::quote_for_list (input) << std::endl;

          BOOST_FOREACH (const boost::filesystem::path& p, state.dependencies())
            {
              stream << dependencies::quote_for_list(p) << std::endl;
            }
        }

      return f;
    }
  }
}

#endif
