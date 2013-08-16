// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <xml/parse/parser.hpp>

#include <xml/parse/error.hpp>
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
#include <xml/parse/type/link.hpp>

#include <xml/parse/util/position.hpp>

#include <fhg/util/join.hpp>
#include <fhg/util/read_bool.hpp>
#include <fhg/util/boost/optional.hpp>

#include <we/mgmt/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/property.hpp>

#include <we2/type/signature.hpp>

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
    namespace
    {
      template<typename T>
        T generic_parse
        ( boost::function<T (const xml_node_type*, state::type&)> parse
        , std::istream& f
        , state::type& state
        , const std::string& name_wanted
        , const std::string& pre
        )
      {
        xml_document_type doc;

        input_type inp (f);

        try
        {
          doc.parse < rapidxml::parse_full
                    | rapidxml::parse_non_destructive
                    > (inp.data());
        }
        catch (const rapidxml::parse_error& e)
        {
          const util::position_type position ( inp.data()
                                             , e.where<char>()
                                             , state.file_in_progress()
                                             );

          std::ostringstream oss;

          oss << "Parse error " << position << ": " << e.what();

          throw rapidxml::parse_error (oss.str().c_str(), e.where<void>());
        }

        state.set_in_progress_position (inp.data());

        xml_node_type* node (doc.first_node());

        if (!node)
        {
          throw error::no_elements_given (pre, state.file_in_progress());
        }

        skip (node, rapidxml::node_declaration);

        const std::string name (name_element (node, state));

        if (!node)
        {
          throw error::no_elements_given (pre, state.file_in_progress());
        }

        if (name != name_wanted)
        {
          state.warn
            (warning::unexpected_element (name, pre, state.file_in_progress()));
        }

        xml_node_type* sib (node->next_sibling());

        skip (sib, rapidxml::node_comment);

        if (sib)
        {
          throw error::more_than_one_definition (pre, state.position (sib));
        }

        return parse (node, state);
      }

      // ******************************************************************* //

      id::ref::function function_type (const xml_node_type*, state::type&);
      id::ref::tmpl tmpl_type (const xml_node_type*, state::type&);

      void property_map_type ( we::type::property::type&
                             , const xml_node_type*
                             , state::type&
                             );
      we::type::property::type
        property_maps_type (const xml_node_type*, state::type&);

      type::structure_type struct_type (const xml_node_type*, state::type&);
      type::structs_type structs_type (const xml_node_type*, state::type&);

      // ******************************************************************* //

      template<typename return_type>
        return_type generic_include
        ( const std::string& file
        , state::type& state
        , boost::function<return_type (const xml_node_type*, state::type&)> fun
        , const std::string& wanted
        , const std::string& pre
        )
      {
        return state.generic_include<return_type>
          ( boost::bind (generic_parse<return_type>, fun, _1, _2, wanted, pre)
          , file
          );
      }

      id::ref::function function_include
        (const std::string& file, state::type& state)
      {
        return generic_include<id::ref::function>
          (file, state, function_type, "defun", "parse_function");
      }

      id::ref::tmpl template_include
        (const std::string& file, state::type& state)
      {
        return generic_include<id::ref::tmpl>
          (file, state, tmpl_type, "template", "parse_template");
      }

      type::structs_type structs_include
        (const std::string& file, state::type& state)
      {
        return generic_include<type::structs_type>
          (file, state, structs_type, "structs", "parse_structs");
      }

      we::type::property::type properties_include
        (const std::string& file, state::type& state)
      {
        return generic_include<we::type::property::type>
          (file, state, property_maps_type, "props", "parse_props");
      }

      // ******************************************************************* //

      void require_type ( type::requirements_type& requirements
                        , const xml_node_type* node
                        , state::type& state
                        )
      {
        const std::string key (required ("require_type", node, "key", state));
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

      // ******************************************************************* //

      void set_type_map ( const xml_node_type* node
                        , const state::type& state
                        , type::type_map_type& map
                        )
      {
        const std::string replace
          (required ("set_type_map", node, "replace", state));
        const std::string with (required ("set_type_map", node, "with", state));

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

      // ******************************************************************* //

      void set_type_get ( const xml_node_type* node
                        , const state::type& state
                        , type::type_get_type& set
                        )
      {
        const std::string name (required ("set_type_get", node, "name", state));

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

      // ******************************************************************* //

      id::ref::connect connect_type ( const xml_node_type* node
                                    , state::type& state
                                    , const petri_net::edge::type& direction
                                    )
      {
        we::type::property::type properties;

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

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
                , properties_include
                  (required ("connect_type", child, "href", state), state)
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
          , boost::none
          , state.position (node)
          , required ("connect_type", node, "place", state)
          , required ("connect_type", node, "port", state)
          , direction
          , properties
          ).make_reference_id();
      }

      // **************************************************************** //

      id::ref::place_map
        place_map_type (const xml_node_type* node, state::type& state)
      {
        we::type::property::type properties;

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

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
                , properties_include
                  (required ("place_map_type", child, "href", state), state)
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
          , boost::none
          , state.position (node)
          , required ("place_map_type", node, "virtual", state)
          , required ("place_map_type", node, "real", state)
          , properties
          ).make_reference_id();
      }

      // **************************************************************** //

      id::ref::port port_type ( const xml_node_type* node
                              , state::type& state
                              , const we::type::PortDirection& direction
                              )
      {
        we::type::property::type properties;

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

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
                , properties_include
                  (required ("port_type", child, "href", state), state)
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
          , boost::none
          , state.position (node)
          , validate_name
            ( validate_prefix ( required ("port_type", node, "name", state)
                              , "port"
                              , state.file_in_progress()
                              )
            , "port"
            , state.file_in_progress()
            )
          , required ("port_type", node, "type", state)
          , optional (node, "place")
          , direction
          , properties
          ).make_reference_id();
      }

      // ******************************************************************* //

      id::ref::transition
        transition_type (const xml_node_type* node, state::type& state)
      {
        const id::transition id (state.id_mapper()->next_id());

        const id::ref::transition transition
          ( type::transition_type
            ( id
            , state.id_mapper()
            , boost::none
            , state.position (node)
            , validate_name ( validate_prefix ( required ( "transition_type"
                                                         , node
                                                         , "name"
                                                         , state
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
            ).make_reference_id()
          );

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "include-function")
            {
              const std::string file
                (required ("transition_type", child, "href", state));

              transition.get_ref().function_or_use
                (function_include (file, state));
            }
            else if (child_name == "use")
            {
              transition.get_ref().function_or_use
                ( type::use_type ( id::use (state.id_mapper()->next_id())
                                 , state.id_mapper()
                                 , id
                                 , state.position (child)
                                 , required
                                   ("transition_type", child, "name", state)
                                 ).make_reference_id()
                );
            }
            else if (child_name == "defun")
            {
              transition.get_ref().function_or_use (function_type (child, state));
            }
            else if (child_name == "place-map")
            {
              transition.get_ref().push_place_map (place_map_type (child, state));
            }
            else if (child_name == "connect-in")
            {
              transition.get_ref().push_connection
                (connect_type (child, state, petri_net::edge::PT));
            }
            else if (child_name == "connect-out")
            {
              transition.get_ref().push_connection
                (connect_type (child, state, petri_net::edge::TP));
            }
            else if (child_name == "connect-inout")
            {
              const id::ref::connect connection_in
                (connect_type (child, state, petri_net::edge::PT));
              const id::ref::connect connection_out
                (connection_in.get().clone());
              connection_out.get_ref().direction (petri_net::edge::TP);

              transition.get_ref().push_connection (connection_in);
              transition.get_ref().push_connection (connection_out);
            }
            else if (child_name == "connect-read")
            {
              transition.get_ref().push_connection
                (connect_type (child, state, petri_net::edge::PT_READ));
            }
            else if (child_name == "condition")
            {
              transition.get_ref().add_conditions (parse_cdata (child, state));
            }
            else if (child_name == "require")
            {
              require_type (transition.get_ref().requirements, child, state);
            }
            else if (child_name == "properties")
            {
              property_map_type (transition.get_ref().properties(), child, state);
            }
            else if (child_name == "include-properties")
            {
              const we::type::property::type deeper
                ( properties_include ( required ( "transition_type"
                                                , child
                                                , "href"
                                                , state
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

      // ******************************************************************* //

      id::ref::specialize
        specialize_type (const xml_node_type* node, state::type& state)
      {
        type::type_map_type type_map;
        type::type_get_type type_get;

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

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
          , boost::none
          , state.position (node)
          , required ("specialize_type", node, "name", state)
          , required ("specialize_type", node, "use", state)
          , type_map
          , type_get
          ).make_reference_id();
      }

      // ******************************************************************* //

      void property_dive ( const xml_node_type* node
                         , state::type& state
                         , we::type::property::type& prop
                         )
      {
        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "property")
            {
              const std::string key
                (required ("propery_dive", child, "key", state));
              const boost::optional<std::string>
                value (optional (child, "value"));

              const std::list<std::string> cdata (parse_cdata (child, state));

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

                  util::property::set_state (state, prop, state.prop_path());
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
              const std::string name
                (required ("property_dive", child, "name", state));

              state.prop_path().push_back (name);

              property_dive (child, state, prop);

              state.prop_path().pop_back ();
            }
            else if (child_name == "include-properties")
            {
              const we::type::property::type deeper
                ( properties_include
                  (required ("property_dive", child, "href", state), state)
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

      void property_map_type ( we::type::property::type& prop
                             , const xml_node_type* node
                             , state::type& state
                             )
      {
        if (!state.ignore_properties())
        {
          const std::string name
            (required ("property_map_type", node, "name", state));

          state.prop_path().push_back (name);

          property_dive (node, state, prop);

          state.prop_path().pop_back ();
        }
      }

      // ******************************************************************* //

      type::structs_type structs_type ( const xml_node_type* node
                                      , state::type& state
                                      )
      {
        type::structs_type v;

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "struct")
            {
              v.push_back (struct_type (child, state));
            }
            else if (child_name == "include-structs")
            {
              const type::structs_type structs
                ( structs_include
                  (required ("structs_type", child, "href", state), state)
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

      // ******************************************************************* //

      we::type::property::type
        property_maps_type (const xml_node_type* node, state::type& state)
      {
        we::type::property::type prop;

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "properties")
            {
              property_map_type (prop, child, state);
            }
            else if (child_name == "include-properties")
            {
              const we::type::property::type deeper
                ( properties_include
                  (required ("property_maps_type", child, "href", state), state)
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

      // ******************************************************************* //

      id::ref::tmpl tmpl_type (const xml_node_type* node, state::type& state)
      {
        boost::optional<id::ref::function> fun;
        type::tmpl_type::names_type template_parameter;
        boost::optional<std::string> name (optional (node, "name"));

        const id::tmpl id (state.id_mapper()->next_id());

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "template-parameter")
            {
              const std::string tn
                (required ("template-parameter", child, "type", state));

              if (template_parameter.find (tn) != template_parameter.end())
              {
                state.warn ( warning::duplicate_template_parameter
                             (name, tn, state.file_in_progress())
                           );
              }

              template_parameter.insert (tn);
            }
            else if (child_name == "defun")
            {
              fun = function_type (child, state);
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
            (name, state.file_in_progress());
        }

        return type::tmpl_type
          ( id
          , state.id_mapper()
          , boost::none
          , state.position (node)
          , name
          , template_parameter
          , *fun
          ).make_reference_id();
      }

      // ******************************************************************* //

      namespace
      {
        boost::tuple< std::string
                    , boost::optional<std::string>
                    , std::list<std::string>
                    >
        parse_function_signature ( const std::string& input
                                 , const std::string& _name
                                 , const util::position_type& pod
                                 )
        {
          // implement the grammar
          // S -> R F A
          // F -> valid_name
          // R -> eps | valid_name
          // A -> eps | '(' L ')' | '(' ')'
          // L -> valid_name | valid_name ',' L
          //
          // here R stands for the return port, F for the function
          // name and A for the list of argument ports

          fhg::util::parse::position pos (input);

          std::string function (parse_name (pos));
          boost::optional<std::string> port_return;
          std::list<std::string> port_arg;

          if (!pos.end())
          {
            if (*pos != '(')
            {
              port_return = function;
              function = parse_name (pos);
            }

            if (!pos.end())
            {
              if (*pos != '(')
              {
                throw error::parse_function::expected ( _name
                                                      , input
                                                      , "("
                                                      , pos()
                                                      , pod.path()
                                                      );
              }

              ++pos;

              while (!pos.end() && *pos != ')')
              {
                port_arg.push_back (parse_name (pos));

                if (!pos.end() && *pos != ')')
                {
                  if (*pos != ',')
                  {
                    throw error::parse_function::expected ( _name
                                                          , input
                                                          , ","
                                                          , pos()
                                                          , pod.path()
                                                          );
                  }

                  ++pos;
                }
              }

              if (pos.end() || *pos != ')')
              {
                throw error::parse_function::expected ( _name
                                                      , input
                                                      , ")"
                                                      , pos()
                                                      , pod.path()
                                                      );
              }

              ++pos;
            }

            while (!pos.end() && isspace(*pos))
            {
              ++pos;
            }

            if (!pos.end())
            {
              throw error::parse_function::expected ( _name
                                                    , input
                                                    , "<end of input>"
                                                    , pos()
                                                    , pod.path()
                                                    );
            }
          }

          return boost::make_tuple (function, port_return, port_arg);
        }
      }

      id::ref::module module_type ( const xml_node_type* node
                                  , state::type& state
                                  )
      {
        const id::module id (state.id_mapper()->next_id());
        const std::string name (required ("module_type", node, "name", state));
        const std::string signature
          (required ("module_type", node, "function", state));
        const util::position_type pod (state.position (node));
        const boost::tuple
          < std::string
          , boost::optional<std::string>
          , std::list<std::string>
          > sig (parse_function_signature (signature, name, pod));
        const std::string function (sig.get<0>());
        const boost::optional<std::string> port_return (sig.get<1>());
        const std::list<std::string> port_arg (sig.get<2>());

        boost::optional<std::string> code;
        boost::optional<util::position_type> pod_of_code;
        std::list<std::string> cincludes;
        std::list<std::string> ldflags;
        std::list<std::string> cxxflags;
        std::list<type::link_type> links;

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "cinclude")
            {
              cincludes.push_back
                (required ("module_type", child, "href", state));
            }
            else if (child_name == "ld")
            {
              ldflags.push_back
                (required ("module_type", child, "flag", state));
            }
            else if (child_name == "cxx")
            {
              cxxflags.push_back
                (required ("module_type", child, "flag", state));
            }
            else if (child_name == "link")
            {
              links.push_back
                ( type::link_type
                  ( required ("module_type", child, "href", state)
                  , optional (child, "prefix")
                  )
                );
            }
            else if (child_name == "code")
            {
              pod_of_code = state.position (child);
              code = fhg::util::join (parse_cdata (child, state), "\n");
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

        return type::module_type
          ( id
          , state.id_mapper()
          , boost::none
          , pod
          , name
          , function
          , port_return
          , port_arg
          , code
          , pod_of_code
          , cincludes
          , ldflags
          , cxxflags
          , links
          ).make_reference_id();
      }

      // ******************************************************************* //

      void parse_token ( const xml_node_type* node
                       , state::type& state
                       , type::place_type& place
                       )
      {
        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "value")
            {
              place.push_token (std::string ( child->value()
                                            , child->value_size()
                                            )
                               );
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
      }

      // ******************************************************************* //

      id::ref::place place_type (const xml_node_type* node, state::type& state)
      {
        const id::place id (state.id_mapper()->next_id());

        const std::string name (required ("place_type", node, "name", state));

        const id::ref::place place
          ( type::place_type
            ( id
            , state.id_mapper()
            , boost::none
            , state.position (node)
            , validate_name ( validate_prefix ( name
                                              , "place"
                                              , state.file_in_progress()
                                              )
                            , "place"
                            , state.file_in_progress()
                            )
            , required ("place_type", node, "type", state)
            , fhg::util::boost::fmap<std::string, bool>
              (fhg::util::read_bool, optional (node, "virtual"))
            ).make_reference_id()
          );

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "token")
            {
              parse_token (child, state, place.get_ref());
            }
            else if (child_name == "properties")
            {
              property_map_type (place.get_ref().properties(), child, state);
            }
            else if (child_name == "include-properties")
            {
              const we::type::property::type deeper
                ( properties_include
                  (required ("place_type", child, "href", state), state)
                );

              util::property::join (state, place.get_ref().properties(), deeper);
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

      // ******************************************************************* //

      id::ref::net net_type (const xml_node_type* node, state::type& state)
      {
        const id::net id (state.id_mapper()->next_id());

        const id::ref::net net
          ( type::net_type
            ( id
            , state.id_mapper()
            , boost::none
            , state.position (node)
            ).make_reference_id()
          );

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "template")
            {
              net.get_ref().push_template (tmpl_type (child, state));
            }
            else if (child_name == "specialize")
            {
              net.get_ref().push_specialize (specialize_type (child, state));
            }
            else if (child_name == "place")
            {
              net.get_ref().push_place (place_type (child, state));
            }
            else if (child_name == "transition")
            {
              net.get_ref().push_transition (transition_type (child, state));
            }
            else if (child_name == "struct")
            {
              net.get_ref().structs.push_back (struct_type (child, state));
            }
            else if (child_name == "include-structs")
            {
              std::cerr << "TODO: Deprecate and eliminate net::include-structs.\n";
              //! \todo deprecate and eliminate
              const type::structs_type structs
                ( structs_include ( required ("net_type", child, "href", state)
                                  , state
                                  )
                );

              net.get_ref().structs.insert ( net.get_ref().structs.end()
                                           , structs.begin()
                                           , structs.end()
                                           );
            }
            else if (child_name == "include-template")
            {
              const std::string file
                (required ("net_type", child, "href", state));
              const boost::optional<std::string> as (optional (child, "as"));

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
                throw error::top_level_anonymous_template (file, "net_type");
              }

              net.get_ref().push_template (tmpl);
            }
            else if (child_name == "properties")
            {
              property_map_type (net.get_ref().properties(), child, state);
            }
            else if (child_name == "include-properties")
            {
              const we::type::property::type deeper
                ( properties_include
                  (required ("net_type", child, "href", state), state)
                );

              util::property::join (state, net.get_ref().properties(), deeper);
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

      // ******************************************************************* //

      id::ref::function
        function_type (const xml_node_type* node, state::type& state)
      {
        const id::function id (state.id_mapper()->next_id());

        const id::ref::function function
          ( type::function_type
            ( id
            , state.id_mapper()
            , boost::none
            , state.position (node)
            , optional (node, "name")
            , fhg::util::boost::fmap<std::string, bool>
              (fhg::util::read_bool, optional (node, "internal"))
            //! \todo see Issue #118 and forbid more than one expression
            , type::expression_type ( state.id_mapper()->next_id()
                                    , state.id_mapper()
                                    , id
                                    , state.position (node)
                                    ).make_reference_id()
            ).make_reference_id()
          );

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "in")
            {
              function.get_ref().push_port
                (port_type (child, state, we::type::PORT_IN));
            }
            else if (child_name == "out")
            {
              function.get_ref().push_port
                (port_type (child, state, we::type::PORT_OUT));
            }
            else if (child_name == "inout")
            {
              const id::ref::port port_in
                (port_type (child, state, we::type::PORT_IN));

              const id::ref::port port_out (port_in.get().clone());
              port_out.get_ref().direction (we::type::PORT_OUT);

              function.get_ref().push_port (port_in);
              function.get_ref().push_port (port_out);
            }
            else if (child_name == "tunnel")
            {
              function.get_ref().push_port
                (port_type (child, state, we::type::PORT_TUNNEL));
            }
            else if (child_name == "struct")
            {
              function.get_ref().structs.push_back (struct_type (child, state));
            }
            else if (child_name == "include-structs")
            {
              const type::structs_type structs
                ( structs_include
                  (required ("function_type", child, "href", state), state)
                );

              function.get_ref().structs.insert ( function.get_ref().structs.end()
                                                , structs.begin()
                                                , structs.end()
                                                );
            }
            else if (child_name == "expression")
            {
              function.get_ref().add_expression (parse_cdata (child, state));
            }
            else if (child_name == "module")
            {
              function.get_ref().content (module_type (child, state));
            }
            else if (child_name == "net")
            {
              function.get_ref().content (net_type (child, state));
            }
            else if (child_name == "condition")
            {
              function.get_ref().add_conditions (parse_cdata (child, state));
            }
            else if (child_name == "properties")
            {
              property_map_type (function.get_ref().properties(), child, state);
            }
            else if (child_name == "include-properties")
            {
              const we::type::property::type deeper
                ( properties_include
                  (required ("function_type", child, "href", state), state)
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

      // ******************************************************************* //

      pnet::type::signature::structure_type
        structure_type (const xml_node_type*, state::type&);

      pnet::type::signature::structured_type
        structured_type (const xml_node_type* node, state::type& state)
      {
        return std::make_pair
          ( validate_field_name
            ( required ("structured_type", node, "name", state)
            , state.file_in_progress()
            )
          , structure_type (node, state)
          );
      }

      pnet::type::signature::structure_type
        structure_type (const xml_node_type* node, state::type& state)
      {
        pnet::type::signature::structure_type s;

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "field")
            {
              s.push_back ( std::make_pair
                            ( required ("struct_field", child, "name", state)
                            , required ("struct_field", child, "type", state)
                            )
                          );
            }
            else if (child_name == "struct")
            {
              s.push_back (structured_type (child, state));
            }
            else
            {
              state.warn
                ( warning::unexpected_element ( child_name
                                              , "structure_type"
                                              , state.file_in_progress()
                                              )
                );
            }
          }
        }

        return s;
      }

      type::structure_type
        struct_type (const xml_node_type* node, state::type& state)
      {
        return type::structure_type
          ( id::structure (state.id_mapper()->next_id())
          , state.id_mapper()
          , boost::none
          , state.position (node)
          , structured_type (node, state)
          );
      }
    }

    // ********************************************************************* //

    id::ref::function just_parse (state::type& state, const std::string& input)
    {
      state.set_input (input);

      return state.generic_parse<id::ref::function>
        ( boost::bind ( generic_parse<id::ref::function>
                      , function_type, _1, _2
                      , "defun", "parse_function"
                      )
        , input
        );
    }

    void post_processing_passes ( const id::ref::function& function
                                , state::type* state
                                )
    {
      // set all the collected requirements to the top level function
      function.get_ref().requirements = state->requirements();

      function.get_ref().specialize (*state);

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

      boost::unordered_set<std::string> structnames;

      type::struct_to_cpp (state, function, structnames);

      type::mk_makefile (state, m, structnames);
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

      type::dump::dump (s, function.get());
    }

    we::mgmt::type::activity_t xml_to_we
      ( const xml::parse::id::ref::function& function
      , const xml::parse::state::type& state
      )
    {
      if (not function.get().name())
      {
        state.warn (warning::synthesize_anonymous_function (function));

        function.get_ref().name (boost::optional<std::string>("anonymous"));
      }

      we::type::transition_t trans
        (function.get_ref().synthesize (*function.get().name(), state));

      we::type::optimize::optimize (trans, state.options_optimize());

      return we::mgmt::type::activity_t (trans);
    }
  } // namespace parse
} // namespace xml
