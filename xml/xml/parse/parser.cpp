// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <xml/parse/parser.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/headergen.hpp>
#include <xml/parse/headerlist.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/util.hpp>
#include <xml/parse/warning.hpp>

#include <xml/parse/id/types.hpp>

#include <xml/parse/rapidxml/types.hpp>

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
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/use.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/read_bool.hpp>
#include <fhg/util/boost/optional.hpp>

#include <we/mgmt/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/property.hpp>
#include <we/type/signature.hpp>

#include <istream>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

// ************************************************************************* //

namespace xml
{
  namespace parse
  {
    template<typename T>
    T generic_parse
      ( boost::function<T (const xml_node_type *, state::type &)> parse
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
    }

    // ********************************************************************* //

    id::ref::function function_type ( const xml_node_type *
                                    , state::type &
                                    , const boost::optional<type::function_type::parent_id_type>& parent
                                    );
    id::ref::module module_type ( const xml_node_type *
                                , state::type &
                                , const id::function& parent
                                );
    id::ref::tmpl tmpl_type ( const xml_node_type *
                            , state::type &
                            );
    id::ref::net net_type ( const xml_node_type *
                          , state::type &
                          , const id::function& parent
                          );
    id::ref::place place_type (const xml_node_type*, state::type&);
    id::ref::port port_type ( const xml_node_type *
                            , state::type &
                            , const id::function& parent
                            , const we::type::PortDirection&
                            );
    void gen_struct_type ( const xml_node_type *, state::type &
                         , signature::desc_t &
                         );
    void substruct_type ( const xml_node_type *, state::type &
                        , signature::desc_t &
                        );
    type::structure_type struct_type ( const xml_node_type *
                               , state::type &
                               , const id::function& parent
                               );
    id::ref::transition transition_type ( const xml_node_type *
                                        , state::type &
                                        , const id::net& parent
                                        );
    id::ref::specialize specialize_type ( const xml_node_type *
                                        , state::type &
                                        , const id::net& parent
                                        );
    void property_map_type ( we::type::property::type &
                           , const xml_node_type *
                           , state::type &
                           );
    we::type::property::type
    property_maps_type (const xml_node_type *, state::type &);

    type::structs_type structs_type ( const xml_node_type *
                                    , state::type & state
                                    , const id::function& parent
                                    );

    namespace
    {
      id::ref::function
      parse_function
        ( std::istream & f
        , state::type & state
        , const boost::optional<type::function_type::parent_id_type>& parent
        )
      {
        return generic_parse<id::ref::function>
          ( boost::bind (function_type, _1, _2, parent)
          , f
          , state
          , "defun"
          , "parse_function"
          );
      }

      id::ref::tmpl parse_template (std::istream& f, state::type& state)
      {
        return generic_parse<id::ref::tmpl>
          (tmpl_type, f, state, "template", "parse_template");
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

    id::ref::function
    function_include
      ( const std::string & file
      , state::type & state
      , const boost::optional<type::function_type::parent_id_type>& parent
      )
    {
      return state.generic_include<id::ref::function>
        (boost::bind (parse_function, _1, _2, parent), file);
    }

    id::ref::tmpl template_include (const std::string& file, state::type& state)
    {
      return state.generic_include<id::ref::tmpl> (parse_template, file);
    }

    type::structs_type structs_include ( const std::string & file
                                              , state::type & state
                                              , const id::function& parent
                                              )
    {
      return state.generic_include<type::structs_type>
        (boost::bind (parse_structs, _1, _2, parent), file);
    }

    we::type::property::type
    properties_include (const std::string & file, state::type & state)
    {
      return
        state.generic_include<we::type::property::type> (parse_props, file);
    }

    // ********************************************************************* //

    void
    require_type ( type::requirements_type & requirements
                 , const xml_node_type * node
                 , state::type & state
                 )
    {
      const std::string key
        (required ("require_type", node, "key", state.file_in_progress()));
      const boost::optional<bool> mmandatory
        ( fhg::util::boost::fmap<std::string, bool>( fhg::util::read_bool
                                                   , optional (node, "mandatory")
                                                   )
        );
      const bool mandatory (mmandatory ? *mmandatory : true);

      requirements.set (key, mandatory);

      // collect all the requirements for the top level function
      state.set_requirement (key, mandatory);
    }

    // ********************************************************************* //

    void
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

    void
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

    namespace
    {
      id::ref::connect connect_type ( const xml_node_type * node
                                    , state::type & state
                                    , const id::transition& parent
                                    , const petri_net::edge::type& direction
                                    )
      {
        we::type::property::type properties;

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
              property_map_type (properties, child, state);
            }
            else if (child_name == "include-properties")
            {
              util::property::join
                ( state
                , properties
                , properties_include ( required ( "connect_type"
                                                , child
                                                , "href"
                                                , state.file_in_progress()
                                                )
                                     , state
                                     )
                );
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

        return type::connect_type
          ( state.id_mapper()->next_id()
          , state.id_mapper()
          , parent
          , required ("connect_type", node, "place", state.file_in_progress())
          , required ("connect_type", node, "port", state.file_in_progress())
          , direction
          , properties
          ).make_reference_id();
      }

      // **************************************************************** //

      id::ref::place_map place_map_type ( const xml_node_type * node
                                        , state::type & state
                                        , const id::transition& parent
                                        )
      {
        we::type::property::type properties;

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
              property_map_type (properties, child, state);
            }
            else if (child_name == "include-properties")
            {
              util::property::join
                ( state
                , properties
                , properties_include ( required ( "place_map_type"
                                                , child
                                                , "href"
                                                , state.file_in_progress()
                                                )
                                     , state
                                     )
                );
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

        return type::place_map_type
          ( state.id_mapper()->next_id()
          , state.id_mapper()
          , parent
          , required ("place_map_type", node, "virtual", state.file_in_progress())
          , required ("place_map_type", node, "real", state.file_in_progress())
          , properties
          ).make_reference_id();
      }
    }

    // ********************************************************************* //

    id::ref::transition
    transition_type ( const xml_node_type * node
                    , state::type & state
                    , const id::net& parent
                    )
    {
      const id::transition id (state.id_mapper()->next_id());

      const id::ref::transition transition
        ( type::transition_type
          ( id
          , state.id_mapper()
          , parent
          , validate_name ( validate_prefix ( required ( "transition_type"
                                                     , node
                                                     , "name"
                                                     , state.file_in_progress()
                                                     )
                                          , "transition"
                                          , state.file_in_progress()
                                          )
                        , "transition"
                        , state.file_in_progress()
                        )
          , fhg::util::boost::fmap<std::string, petri_net::priority_type>
            ( boost::lexical_cast<petri_net::priority_type>
            , optional (node, "priority")
            )
          , fhg::util::boost::fmap<std::string, bool>
            (fhg::util::read_bool, optional (node, "inline"))
          , fhg::util::boost::fmap<std::string, bool>
            (fhg::util::read_bool, optional (node, "internal"))
          , state.file_in_progress()
          ).make_reference_id()
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



                  transition.get_ref().function_or_use
                    ( function_include
                      ( file
                      , state
                      , type::function_type::make_parent (id)
                      )
                    );
                }
              else if (child_name == "use")
                {
                  transition.get_ref().function_or_use
                    ( type::use_type ( id::use (state.id_mapper()->next_id())
                                     , state.id_mapper()
                                     , id
                                     , required ( "transition_type"
                                                , child
                                                , "name"
                                                , state.file_in_progress()
                                                )
                                     ).make_reference_id()
                    );
                }
              else if (child_name == "defun")
                {
                  transition.get_ref().function_or_use
                    ( function_type
                      ( child
                      , state
                      , type::function_type::make_parent (id)
                      )
                    );
                }
              else if (child_name == "place-map")
                {
                  transition.get_ref()
                    .push_place_map (place_map_type (child, state, id));
                }
              else if (child_name == "connect-in")
                {
                  transition.get_ref().push_connection
                    (connect_type (child, state, id, petri_net::edge::PT));
                }
              else if (child_name == "connect-out")
                {
                  transition.get_ref().push_connection
                    (connect_type (child, state, id, petri_net::edge::TP));
                }
              else if (child_name == "connect-inout")
                {
                  const id::ref::connect connection_in
                    (connect_type (child, state, id, petri_net::edge::PT));
                  const id::ref::connect connection_out
                    (connection_in.get().clone (id));
                  connection_out.get_ref().direction (petri_net::edge::TP);

                  transition.get_ref().push_connection (connection_in);
                  transition.get_ref().push_connection (connection_out);
                }
              else if (child_name == "connect-read")
                {
                  transition.get_ref().push_connection
                    (connect_type (child, state, id, petri_net::edge::PT_READ));
                }
              else if (child_name == "condition")
                {
                  const type::conditions_type conds
                    ( parse_cdata<type::conditions_type>
                      ( child
                      , state.file_in_progress()
                      )
                    );

                  transition.get_ref()
                    .cond.insert ( state.id_mapper()->get_ref (id)->cond.end()
                                 , conds.begin()
                                 , conds.end()
                                 );
                }
              else if (child_name == "require")
                {
                  require_type ( transition.get_ref().requirements
                               , child
                               , state
                               );
                }
              else if (child_name == "properties")
                {
                  property_map_type ( transition.get_ref().properties()
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
                                       , transition.get_ref().properties()
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

      return transition;
    }

    // ********************************************************************* //

    id::ref::specialize
    specialize_type ( const xml_node_type * node
                    , state::type & state
                    , const id::net& parent
                    )
    {
      type::type_map_type type_map;
      type::type_get_type type_get;

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
            set_type_map (child, state, type_map);
          }
          else if (child_name == "type-get")
          {
            set_type_get (child, state, type_get);
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

      return type::specialize_type
        ( state.id_mapper()->next_id()
        , state.id_mapper()
        , parent
        , required ("specialize_type", node, "name", state.file_in_progress())
        , required ("specialize_type", node, "use", state.file_in_progress())
        , type_map
        , type_get
        , state.file_in_progress()
        ).make_reference_id();
    }

    // ********************************************************************* //

    void
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
                  const boost::optional<std::string>
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

    void
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
    }

    // ********************************************************************* //

    void token_field_type ( const xml_node_type * node
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

    id::ref::tmpl tmpl_type (const xml_node_type * node, state::type & state)
    {
      boost::optional<id::ref::function> fun;
      type::tmpl_type::names_type template_parameter;
      boost::optional<std::string> name (optional (node, "name"));

      const id::tmpl id (state.id_mapper()->next_id());

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
                  fun = function_type
                    ( child
                    , state
                    , type::function_type::make_parent(id)
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
        , boost::none
        , name
        , template_parameter
        , *fun
        , state.file_in_progress()
        ).make_reference_id();
    }

    // ********************************************************************* //

    id::ref::function
    function_type
      ( const xml_node_type * node
      , state::type & state
      , const boost::optional<type::function_type::parent_id_type>& parent
      )
    {
      const id::function id (state.id_mapper()->next_id());

      const id::ref::function function
        ( type::function_type
          ( id
          , state.id_mapper()
          , parent
          , type::expression_type ( state.id_mapper()->next_id()
                                  , state.id_mapper()
                                  , id
                                  ).make_reference_id()
          ).make_reference_id()
        );

      function.get_ref().path = state.file_in_progress();
      function.get_ref().name (optional (node, "name"));
      function.get_ref().internal =
        fhg::util::boost::fmap<std::string, bool>( fhg::util::read_bool
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
            function.get_ref().push_port
              (port_type (child, state, id, we::type::PORT_IN));
          }
          else if (child_name == "out")
          {
            function.get_ref().push_port
              (port_type (child, state, id, we::type::PORT_OUT));
          }
          else if (child_name == "inout")
          {
            const id::ref::port port_in
              (port_type (child, state, id, we::type::PORT_IN));
            const id::ref::port port_out
              (port_in.get().clone (id));
            port_out.get_ref().direction (we::type::PORT_OUT);
            function.get_ref().push_port (port_in);
            function.get_ref().push_port (port_out);
          }
          else if (child_name == "tunnel")
          {
            function.get_ref().push_port
              (port_type (child, state, id, we::type::PORT_TUNNEL));
          }
          else if (child_name == "struct")
          {
            function.get_ref().structs.push_back (struct_type (child, state, id));
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

            function.get_ref().structs.insert ( function.get_ref().structs.end()
                                              , structs.begin()
                                              , structs.end()
                                              );
          }
          else if (child_name == "expression")
          {
            function.get_ref().add_expression
              ( parse_cdata<type::expressions_type>
                (child, state.file_in_progress())
              );
          }
          else if (child_name == "module")
          {
            function.get_ref().f = module_type (child, state, id);
          }
          else if (child_name == "net")
          {
            function.get_ref().f = net_type (child, state, id);
          }
          else if (child_name == "condition")
          {
            const type::conditions_type conds
              ( parse_cdata<type::conditions_type>
                (child, state.file_in_progress())
              );

            function.get_ref().cond.insert ( function.get_ref().cond.end()
                                           , conds.begin()
                                           , conds.end()
                                           );
          }
          else if (child_name == "properties")
          {
            property_map_type (function.get_ref().properties(), child, state);
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

            util::property::join (state, function.get_ref().properties(), deeper);
          }
          else if (child_name == "require")
          {
            require_type (function.get_ref().requirements, child, state);
          }
          else
          {
            state.warn ( warning::unexpected_element ( child_name
                                                     , "function_type"
                                                     , state.file_in_progress()
                                                     )
                       );
          }
        }
      }

      return function;
    }

    // ********************************************************************* //

    id::ref::module
    module_type ( const xml_node_type * node
                , state::type & state
                , const id::function& parent
                )
    {
      const id::module id (state.id_mapper()->next_id());

      const id::ref::module module
        ( type::module_type
          ( id
          , state.id_mapper()
          , parent
          , required ("module_type", node, "name", state.file_in_progress())
          , required ("module_type", node, "function", state.file_in_progress())
          , state.file_in_progress()
          ).make_reference_id()
        );

      state.id_mapper()->get_ref (id)->path = state.file_in_progress();

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

                  module.get_ref().cincludes.push_back (href);
                }
              else if (child_name == "ld")
                {
                  const std::string flag
                    (required ("module_type", child, "flag", state.file_in_progress()));

                  module.get_ref().ldflags.push_back (flag);
                }
              else if (child_name == "cxx")
                {
                  const std::string flag
                    (required ("module_type", child, "flag", state.file_in_progress()));

                  module.get_ref().cxxflags.push_back (flag);
                }
              else if (child_name == "link")
                {
                  module.get_ref().links.push_back
                    ( required ( "module_type"
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

                  module.get_ref().code = fhg::util::join (cdata, "\n");
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

      return module;
    }

    // ********************************************************************* //

    id::ref::net
      net_type ( const xml_node_type * node
               , state::type & state
               , const id::function& parent
               )
    {
      const id::net id (state.id_mapper()->next_id());

      const id::ref::net net
        ( type::net_type
          ( id
          , state.id_mapper()
          , parent
          , state.file_in_progress()
          ).make_reference_id()
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
              if (child_name == "template")
                {
                  net.get_ref().push_template (tmpl_type (child, state));
                }
              else if (child_name == "specialize")
                {
                  net.get_ref()
                    .push_specialize (specialize_type (child, state, id));
                }
              else if (child_name == "place")
                {
                  net.get_ref().push_place (place_type (child, state));
                }
              else if (child_name == "transition")
                {
                  net.get_ref()
                    .push_transition (transition_type (child, state, id));
                }
              else if (child_name == "struct")
                {
                  net.get_ref()
                    .structs.push_back (struct_type (child, state, parent));
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

                  net.get_ref().structs.insert ( net.get_ref().structs.end()
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
                  const boost::optional<std::string> as
                    (optional (child, "as"));

                  id::ref::tmpl tmpl (template_include (file, state));

                  if (as)
                  {
                    if (tmpl.get().name() && *tmpl.get().name() != *as)
                    {
                      state.warn
                        ( warning::overwrite_template_name_as
                          ( *tmpl.get().name()
                          , *as
                          , state.file_in_progress()
                          )
                        );
                    }

                    tmpl.get_ref().name (*as);
                  }

                  if (not tmpl.get().name())
                  {
                    throw error::top_level_anonymous_template
                      (file, "net_type");
                  }

                  net.get_ref().push_template (tmpl);
                }
              else if (child_name == "properties")
                {
                  property_map_type ( net.get_ref().properties()
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
                                       , net.get_ref().properties()
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

      return net;
    }

    // ********************************************************************* //

    type::place_type::token_type
      parse_token (const xml_node_type* node, state::type& state)
    {
      type::place_type::token_type token;

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
            return std::string (child->value());
          }
          else if (child_name == "field")
          {
            token_field_type (child, state, token);
          }
          else
          {
            state.warn
              ( warning::unexpected_element ( child_name
                                            , "parse_token"
                                            , state.file_in_progress()
                                            )
              );
          }
        }
      }

      return token;
    }

    // ********************************************************************* //

    id::ref::place place_type (const xml_node_type* node, state::type& state)
    {
      const id::place id (state.id_mapper()->next_id());

      const std::string name
        (required ("place_type", node, "name", state.file_in_progress()));

      const id::ref::place place
        ( type::place_type
          ( id
          , state.id_mapper()
          , boost::none
          , validate_name ( validate_prefix ( name
                                            , "place"
                                            , state.file_in_progress()
                                            )
                          , "place"
                          , state.file_in_progress()
                          )
          , required ("place_type", node, "type", state.file_in_progress())
          , fhg::util::boost::fmap<std::string, bool> ( fhg::util::read_bool
                                                      , optional (node, "virtual")
                                                      )
          ).make_reference_id()
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
                  place.get_ref().push_token (parse_token (child, state));
                }
              else if (child_name == "properties")
                {
                  property_map_type ( place.get_ref().properties()
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
                                       , place.get_ref().properties()
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

      return place;
    }

    // ********************************************************************* //

    id::ref::port port_type ( const xml_node_type * node
                            , state::type & state
                            , const id::function& parent
                            , const we::type::PortDirection& direction
                            )
    {
      we::type::property::type properties;

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
            property_map_type (properties, child, state);
          }
          else if (child_name == "include-properties")
          {
            util::property::join
              ( state
              , properties
              , properties_include ( required ( "port_type"
                                              , child
                                              , "href"
                                              , state.file_in_progress()
                                              )
                                   , state
                                   )
              );
          }
          else
          {
            state.warn ( warning::unexpected_element ( child_name
                                                     , "port_type"
                                                     , state.file_in_progress()
                                                     )
                       );
          }
        }
      }

      return type::port_type
        ( state.id_mapper()->next_id()
        , state.id_mapper()
        , parent
        , validate_name
          ( validate_prefix ( required ( "port_type"
                                       , node
                                       , "name"
                                       , state.file_in_progress()
                                       )
                            , "port"
                            , state.file_in_progress()
                            )
          , "port"
          , state.file_in_progress()
          )
        , required ("port_type", node, "type", state.file_in_progress())
        , optional (node, "place")
        , direction
        , properties
        ).make_reference_id();
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
        ( id::structure (state.id_mapper()->next_id())
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

    // ********************************************************************* //

    id::ref::function
    just_parse (state::type & state, const std::string & input)
    {
      state.set_input (input);

      return state.generic_parse<id::ref::function>
        (boost::bind (parse_function, _1, _2, boost::none), input);
    }

    void post_processing_passes ( const id::ref::function& function
                                , state::type* state
                                )
    {
      // set all the collected requirements to the top level function
      function.get_ref().requirements = state->requirements();

      function.get_ref().specialize (*state);

      function.get_ref().resolve (*state, function.get().forbidden_below());

      function.get_ref().type_check (*state);
      function.get_ref().sanity_check (*state);
    }

    void generate_cpp ( const id::ref::function& function
                      , const state::type& state
                      )
    {
      type::fun_info_map m;

      type::find_module_calls (state, function, m);

      type::mk_wrapper (state, m);
      type::mk_makefile (state, m);

      includes::descrs_type descrs;

      includes::mks (descrs);
      includes::we_header_gen (state, descrs);

      type::struct_to_cpp (state, function);
    }

    void dump_xml ( const id::ref::function& function
                  , const state::type& state
                  )
    {
      const std::string& file (state.dump_xml_file());

      std::ofstream stream (file.c_str());
      if (!stream)
      {
        throw error::could_not_open_file (file);
      }

      fhg::util::xml::xmlstream s (stream);

      type::dump::dump (s, function.get(), state);
    }

    we::mgmt::type::activity_t xml_to_we
      ( const xml::parse::id::ref::function& function
      , const xml::parse::state::type& state
      )
    {
      we::type::transition_t trans (function.get_ref().synthesize (state));

      we::type::optimize::optimize (trans, state.options_optimize());

      return trans;
    }
  } // namespace parse
} // namespace xml
