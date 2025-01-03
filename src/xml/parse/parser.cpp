// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <xml/parse/parser.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/rapidxml/types.hpp>
#include <xml/parse/state.hpp>
#include <xml/parse/type/connect.hpp>
#include <xml/parse/type/eureka.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/memory_buffer.hpp>
#include <xml/parse/type/memory_transfer.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/multi_mod.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/place.hpp>
#include <xml/parse/type/place_map.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type/preferences.hpp>
#include <xml/parse/type/response.hpp>
#include <xml/parse/type/specialize.hpp>
#include <xml/parse/type/struct.hpp>
#include <xml/parse/type/template.hpp>
#include <xml/parse/type/transition.hpp>
#include <xml/parse/type/use.hpp>
#include <xml/parse/util/cdata.hpp>
#include <xml/parse/util/expect.hpp>
#include <xml/parse/util/name_element.hpp>
#include <xml/parse/util/optional.hpp>
#include <xml/parse/util/position.hpp>
#include <xml/parse/util/property.hpp>
#include <xml/parse/util/required.hpp>
#include <xml/parse/util/skip.hpp>
#include <xml/parse/util/unique.hpp>
#include <xml/parse/util/valid_name.hpp>
#include <xml/parse/util/validprefix.hpp>
#include <xml/parse/util/validstructfield.hpp>
#include <xml/parse/warning.hpp>

#include <we/type/Transition.hpp>
#include <we/type/id.hpp>
#include <we/type/property.hpp>
#include <we/type/signature.hpp>
#include <we/type/value.hpp>
#include <we/type/value/show.hpp>

#include <fhg/util/cctype.hpp>
#include <fhg/util/read_bool.hpp>
#include <util-generic/join.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm.hpp>

#include <FMT/xml/parse/util/position.hpp>
#include <fmt/core.h>
#include <functional>
#include <istream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace
{
  template<typename U>
    ::boost::optional<U> fmap ( U (*f)(std::string const&)
                              , ::boost::optional<std::string> const& m
                              )
  {
    if (m)
    {
      return f (*m);
    }

    return {};
  }
}

// ************************************************************************* //

namespace xml
{
  namespace parse
  {
    namespace
    {
      template<typename T>
        T generic_parse
        ( std::function<T (const xml_node_type*, state::type&)> parse
        , std::istream& f
        , state::type& state
        , std::string const& name_wanted
        , std::string const& pre
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
        catch (rapidxml::parse_error const& e)
        {
          throw std::runtime_error
            { fmt::format
              ( "Parse error: {}: {}"
              , util::position_type
                (inp.data(), e.where<char>(), state.file_in_progress())
              , e.what()
              )
            };
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

      type::function_type function_type (const xml_node_type*, state::type&);
      type::tmpl_type tmpl_type (const xml_node_type*, state::type&);

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
        ( std::string const& file
        , state::type& state
        , std::function<return_type (const xml_node_type*, state::type&)> fun
        , std::string const& wanted
        , std::string const& pre
        )
      {
        return state.generic_include<return_type>
          ( std::bind (generic_parse<return_type>, fun, std::placeholders::_1, std::placeholders::_2, wanted, pre)
          , file
          );
      }

      type::function_type function_include
        (std::string const& file, state::type& state)
      {
        return generic_include<type::function_type>
          (file, state, function_type, "defun", "parse_function");
      }

      type::tmpl_type template_include
        (std::string const& file, state::type& state)
      {
        return generic_include<type::tmpl_type>
          (file, state, tmpl_type, "template", "parse_template");
      }

      type::structs_type structs_include
        (std::string const& file, state::type& state)
      {
        return generic_include<type::structs_type>
          (file, state, structs_type, "structs", "parse_structs");
      }

      we::type::property::type properties_include
        (std::string const& file, state::type& state)
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
        requirements.set (key);

        // collect all the requirements for the top level function
        state.set_requirement (key);
      }

      // ******************************************************************* //

      type::preferences_type preferences_type ( const xml_node_type* node
                                              , state::type& state
                                              )
      {
        std::unordered_set<type::preference_type> unique_targets;
        std::list<type::preference_type> target_list;

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "target")
            {
              const std::string target_name
                (validate_name ( std::string (child->value(), child->value_size())
                               , "target"
                               , state.file_in_progress()
                               )
                );

              if (!unique_targets.emplace (target_name).second)
              {
                throw error::duplicate_preference ( target_name
                                                  , state.position (child)
                                                  );
              }
              else
              {
                target_list.push_back (target_name);
              }
            }
            else
            {
              state.warn
                ( warning::unexpected_element ( child_name
                                              , "target"
                                              , state.file_in_progress()
                                              )
                );
            }
          }
        }

        if (target_list.empty())
        {
          throw error::empty_preferences (state.position (node));
        }

        return target_list;
      }

      void is_matching_preferences_and_modules_with_eureka_id
        ( const xml_node_type* node
        , state::type const& state
        , type::preferences_type const& preferences
        , type::multi_module_type const& multi_mod
        )
      {
        ::boost::optional<we::type::eureka_id_type> const& eureka_group
          (multi_mod.eureka_id());

        std::list<type::preference_type> modules;
        for (auto const& mod : multi_mod.modules())
        {
          if (mod.second.eureka_id() != eureka_group)
          {
            throw error::mismatching_eureka_for_module
              ( mod.second.name()
              , state.position (node)
              );
          }
          modules.push_back (mod.first);
        }
        modules.sort();

        std::list<type::preference_type> pref_list (preferences.targets());
        pref_list.sort();

        std::list<type::preference_type> mismatching_preferences;
        std::set_difference ( pref_list.begin()
                            , pref_list.end()
                            , modules.begin()
                            , modules.end()
                            , std::inserter
                              ( mismatching_preferences
                              , mismatching_preferences.begin()
                              )
                            );

        std::list<type::preference_type> mismatching_modules;
        std::set_difference ( modules.begin()
                            , modules.end()
                            , pref_list.begin()
                            , pref_list.end()
                            , std::inserter
                              ( mismatching_modules
                              , mismatching_modules.begin()
                              )
                            );

        if (mismatching_modules.size() || mismatching_preferences.size())
        {
          throw error::mismatching_modules_and_preferences
            ( mismatching_preferences
            , mismatching_modules
            , state.position (node)
            );
        }
      }

      // ******************************************************************* //

      void set_type_map ( const xml_node_type* node
                        , state::type const& state
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
                        , state::type const& state
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

      type::connect_type connect_type ( const xml_node_type* node
                                      , state::type& state
                                      , we::edge::type const& direction
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
          ( state.position (node)
          , required ("connect_type", node, "place", state)
          , required ("connect_type", node, "port", state)
          , direction
          , properties
          );
      }

      // **************************************************************** //

      type::response_type response_type ( const xml_node_type* node
                                        , state::type& state
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
                  (required ("response_type", child, "href", state), state)
                );
            }
            else
            {
              state.warn
                ( warning::unexpected_element ( child_name
                                              , "response_type"
                                              , state.file_in_progress()
                                              )
                );
            }
          }
        }

        return type::response_type
          ( state.position (node)
          , required ("response_type", node, "port", state)
          , required ("response_type", node, "to", state)
          , properties
          );
      }


      // **************************************************************** //

      type::eureka_type eureka_type ( const xml_node_type* node
                                    , state::type& state
                                    )
      {
        return type::eureka_type
          ( state.position (node)
          , required ("eureka_type", node, "port", state)
          );
      }

      // **************************************************************** //

      type::place_map_type
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
          ( state.position (node)
          , required ("place_map_type", node, "virtual", state)
          , required ("place_map_type", node, "real", state)
          , properties
          );
      }

      // **************************************************************** //

      template<typename Memory_Transfer>
        Memory_Transfer memory_transfer
        ( const xml_node_type* node
        , state::type& state
        , std::function<Memory_Transfer ( const xml_node_type*
                                        , state::type const&
                                        , std::string const&
                                        , std::string const&
                                        , we::type::property::type const&
                                        , ::boost::optional<bool> const&
                                        , ::boost::optional<bool> const&
                                        )> make_transfer
        )
      {
        std::string global;
        std::string local;
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
                ( required ("memory_transfer", child, "href", state)
                , state
                )
                );
            }
            else if (child_name == "global")
            {
              global = fhg::util::join (parse_cdata (child, state), ';').string();
            }
            else if (child_name == "local")
            {
              local = fhg::util::join (parse_cdata (child, state), ';').string();
            }
            else
            {
              state.warn
                ( warning::unexpected_element ( child_name
                                              , "memory_transfer"
                                              , state.file_in_progress()
                                              )
                );
            }
          }
        }

        return make_transfer
          ( node
          , state
          , global
          , local
          , properties
          , fmap<bool>
            ( fhg::util::read_bool
            , optional (node, "not-modified-in-module-call")
            )
          , fmap<bool>
              ( fhg::util::read_bool
              , optional (node, "allow-empty-ranges")
              )
          );
      }

      type::memory_get memory_get
        (const xml_node_type* node, state::type& state)
      {
        return memory_transfer<type::memory_get>
          ( node
          , state
          , [] ( const xml_node_type* node_
               , state::type const& state_
               , std::string const& global
               , std::string const& local
               , we::type::property::type const& properties
               , ::boost::optional<bool> const& // ignored
               , ::boost::optional<bool> const& allow_empty_ranges
               )
          { return type::memory_get
              ( state_.position (node_)
              , global
              , local
              , properties
              , allow_empty_ranges
              );
          }
          );
      }

      type::memory_put memory_put
        (const xml_node_type* node, state::type& state)
      {
        return memory_transfer<type::memory_put>
          ( node
          , state
          , [] ( const xml_node_type* node_
               , state::type const& state_
               , std::string const& global
               , std::string const& local
               , we::type::property::type const& properties
               , ::boost::optional<bool> const& not_modified_in_module_call
               , ::boost::optional<bool> const& allow_empty_ranges
               )
          { return type::memory_put ( state_.position (node_)
                                    , global
                                    , local
                                    , properties
                                    , not_modified_in_module_call
                                    , allow_empty_ranges
                                    );
          }
          );
      }

      type::memory_getput memory_getput
        (const xml_node_type* node, state::type& state)
      {
        return memory_transfer<type::memory_getput>
          ( node
          , state
          , [] ( const xml_node_type* node_
               , state::type const& state_
               , std::string const& global
               , std::string const& local
               , we::type::property::type const& properties
               , ::boost::optional<bool> const& not_modified_in_module_call
               , ::boost::optional<bool> const& allow_empty_ranges
               )
          { return type::memory_getput ( state_.position (node_)
                                       , global
                                       , local
                                       , properties
                                       , not_modified_in_module_call
                                       , allow_empty_ranges
                                       );
          }
          );
      }

      type::memory_buffer_type memory_buffer_type ( const xml_node_type* node
                                                  , state::type& state
                                                  )
      {
        std::string const name
          ( validate_name
            ( validate_prefix
              ( required ("memory_buffer_type", node, "name", state)
              , "memory_buffer"
              , state.file_in_progress()
              )
            , "memory_buffer"
            , state.file_in_progress()
            )
          );

        ::boost::optional<std::string> size;
        ::boost::optional<std::string> alignment;

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
                  ( required ("memory_buffer_type", child, "href", state)
                  , state
                  )
                );
            }
            else if (child_name == "size")
            {
              size = fhg::util::join (parse_cdata (child, state), ';').string();
            }
            else if (child_name == "alignment")
            {
              alignment = fhg::util::join (parse_cdata (child, state), ';').string();
            }
            else
            {
              state.warn
                ( warning::unexpected_element ( child_name
                                              , "memory_buffer_type"
                                              , state.file_in_progress()
                                              )
                );
            }
          }
        }

        if (!size)
        {
          throw error::memory_buffer_without_size
            (name, state.position (node));
        }

        return type::memory_buffer_type
          ( state.position (node)
          , name
          , *size
          , alignment.get_value_or ("1UL")
          , fmap<bool>
            (fhg::util::read_bool, optional (node, "read-only"))
          , properties
          );
      }

      type::port_type port_type ( const xml_node_type* node
                                , state::type& state
                                , we::type::PortDirection const& direction
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
          ( state.position (node)
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
          );
      }

      // ******************************************************************* //

      type::transition_type
        transition_type (const xml_node_type* node, state::type& state)
      {
        ::boost::optional<type::function_type> function;
        ::boost::optional<type::use_type> use;
        type::transition_type::connections_type connections;
        type::transition_type::responses_type responses;
        type::transition_type::eurekas_type eurekas;
        type::transition_type::place_maps_type place_map;
        std::list<type::structure_type> structs;
        type::conditions_type conditions;
        type::requirements_type requirements;
        we::type::property::type properties;

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

              function = function_include (file, state);
            }
            else if (child_name == "use")
            {
              use = type::use_type ( state.position (child)
                                   , required
                                       ("transition_type", child, "name", state)
                                   );
            }
            else if (child_name == "defun")
            {
              function = function_type (child, state);
            }
            else if (child_name == "place-map")
            {
              place_map.push<error::duplicate_place_map>
                (place_map_type (child, state));
            }
            else if (child_name == "connect-in")
            {
              connections.push<error::duplicate_connect>
                (connect_type (child, state, we::edge::PT{}));
            }
            else if (child_name == "connect-out")
            {
              connections.push<error::duplicate_connect>
                (connect_type (child, state, we::edge::TP{}));
            }
            else if (child_name == "connect-inout")
            {
              connections.push<error::duplicate_connect>
                (connect_type (child, state, we::edge::PT{}));
              connections.push<error::duplicate_connect>
                (connect_type (child, state, we::edge::TP{}));
            }
            else if (child_name == "connect-out-many")
            {
              connections.push<error::duplicate_connect>
                (connect_type (child, state, we::edge::TP_MANY{}));
            }
            else if (child_name == "connect-read")
            {
              connections.push<error::duplicate_connect>
                (connect_type (child, state, we::edge::PT_READ{}));
            }
            else if (child_name == "connect-number-of-tokens")
            {
              connections.push<error::duplicate_connect>
                (connect_type (child, state, we::edge::PT_NUMBER_OF_TOKENS{}));
            }
            else if (child_name == "connect-response")
            {
              responses.push<error::duplicate_response>
                (response_type (child, state));
            }
            else if (child_name == "connect-eureka")
            {
              eurekas.push<error::duplicate_eureka>
                (eureka_type (child, state));
            }
            else if (child_name == "condition")
            {
              auto const cs (parse_cdata (child, state));

              conditions.insert (conditions.end(), cs.begin(), cs.end());
            }
            else if (child_name == "require")
            {
              require_type (requirements, child, state);
            }
            else if (child_name == "properties")
            {
              property_map_type (properties, child, state);
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

              util::property::join (state, properties, deeper);
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

#define TRANSITION(_function_or_use) type::transition_type                 \
          { state.position (node)                                          \
          , _function_or_use                                               \
          , validate_name ( validate_prefix ( required ( "transition_type" \
                                                       , node              \
                                                       , "name"            \
                                                       , state             \
                                                       )                   \
                                            , "transition"                 \
                                            , state.file_in_progress()     \
                                            )                              \
                          , "transition"                                   \
                          , state.file_in_progress()                       \
                          )                                                \
          , connections                                                    \
          , responses                                                      \
          , eurekas                                                        \
          , place_map                                                      \
          , structs                                                        \
          , conditions                                                     \
          , requirements                                                   \
          , fmap<we::priority_type>                                        \
            ( ::boost::lexical_cast<we::priority_type>                       \
            , optional (node, "priority")                                  \
            )                                                              \
          , fmap<bool>                                                     \
            (fhg::util::read_bool, optional (node, "inline"))              \
          , properties                                                     \
          }

        if (!!function)
        {
          return TRANSITION (function.get());
        }

        if (!!use)
        {
          return TRANSITION (use.get());
        }

        throw std::logic_error("transition requires function or use");

#undef TRANSITION
      }

      // ******************************************************************* //

      type::specialize_type
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
          ( state.position (node)
          , required ("specialize_type", node, "name", state)
          , required ("specialize_type", node, "use", state)
          , type_map
          , type_get
          );
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
              const ::boost::optional<std::string>
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
                  util::property::set_state
                    ( state
                    , prop
                    , state.prop_path()
                    , we::type::property::value_type()
                    );
                }
                else
                {
                  util::property::set_state
                    ( state
                    , prop
                    , state.prop_path()
                    , pnet::type::value::read (cdata.front())
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
                                          , pnet::type::value::read (*value)
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

      type::tmpl_type tmpl_type (const xml_node_type* node, state::type& state)
      {
        ::boost::optional<type::function_type> fun;
        type::tmpl_type::names_type template_parameter;
        ::boost::optional<std::string> name (optional (node, "name"));

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
                                              , "tmpl_type"
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
          ( state.position (node)
          , name
          , template_parameter
          , *fun
          );
      }

      // ******************************************************************* //

      namespace
      {
        std::tuple< std::string
                  , ::boost::optional<std::string>
                  , std::list<std::string>
                  , ::boost::optional<std::string>
                  , std::list<std::string>
                  >
        parse_function_signature
          ( std::string const& input
          , std::string const& _name
          , util::position_type const& pod
          , std::string const& module_name
          , type::function_type::ports_type const& ports
          , fhg::pnet::util::unique<type::memory_buffer_type> const& memory_buffers
          , ::boost::filesystem::path const& path
          )
        {
          auto&& is_known_port
            ( [&ports] (std::string const& name)
              {
                return ports.has ({name, we::type::port::direction::In{}})
                  || ports.has ({name, we::type::port::direction::Out{}});
              }
            );
          auto&& is_known_memory_buffer
            ( [&memory_buffers] (std::string const& name)
              {
                return memory_buffers.has (name);
              }
            );

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
          ::boost::optional<std::string> port_return;
          std::list<std::string> port_arg;
          ::boost::optional<std::string> memory_buffer_return;
          std::list<std::string> memory_buffer_arg;

          if (!pos.end())
          {
            if (*pos != '(')
            {
              std::string const port_or_memory_buffer (function);
              function = parse_name (pos);

              if (is_known_port (port_or_memory_buffer))
              {
                port_return = port_or_memory_buffer;
              }
              else if (is_known_memory_buffer (port_or_memory_buffer))
              {
                memory_buffer_return = port_or_memory_buffer;
              }
              else
              {
                throw error::function_description_with_unknown_port
                  ( "return"
                  , port_or_memory_buffer
                  , module_name
                  , function
                  , path
                  );
              }
            }

            if (!pos.end())
            {
              if (*pos != '(')
              {
                throw error::parse_function::expected ( _name
                                                      , input
                                                      , "("
                                                      , pos.eaten()
                                                      , pod.path()
                                                      );
              }

              ++pos;

              while (!pos.end() && *pos != ')')
              {
                std::string const port_or_memory_buffer (parse_name (pos));

                if (is_known_port (port_or_memory_buffer))
                {
                  port_arg.push_back (port_or_memory_buffer);
                }
                else if (is_known_memory_buffer (port_or_memory_buffer))
                {
                  memory_buffer_arg.push_back (port_or_memory_buffer);
                }
                else
                {
                  throw error::function_description_with_unknown_port
                    ( "argument"
                    , port_or_memory_buffer
                    , module_name
                    , function
                    , path
                    );
                }

                if (!pos.end() && *pos != ')')
                {
                  if (*pos != ',')
                  {
                    throw error::parse_function::expected ( _name
                                                          , input
                                                          , ","
                                                          , pos.eaten()
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
                                                      , pos.eaten()
                                                      , pod.path()
                                                      );
              }

              ++pos;
            }

            while (!pos.end() && fhg::util::isspace (*pos))
            {
              ++pos;
            }

            if (!pos.end())
            {
              throw error::parse_function::expected ( _name
                                                    , input
                                                    , "<end of input>"
                                                    , pos.eaten()
                                                    , pod.path()
                                                    );
            }
          }

          return std::make_tuple ( function
                                 , port_return
                                 , port_arg
                                 , memory_buffer_return
                                 , memory_buffer_arg
                                 );
        }
      }

      type::module_type module_type
        ( const xml_node_type* node
        , state::type& state
        , type::function_type::ports_type const& ports
        , fhg::pnet::util::unique<type::memory_buffer_type> const& memory_buffers
        , bool is_target_required
        , ::boost::filesystem::path const& path
        )
      {
        const std::string name
          (validate_name ( required ("module_type", node, "name", state)
                         , "module_type"
                         , state.file_in_progress()
                         )
          );
        const std::string signature
          (required ("module_type", node, "function", state));
        const ::boost::optional<bool> pass_context
          (fmap<bool>
          (fhg::util::read_bool, optional (node, "pass_context")));
        const ::boost::optional<bool> require_function_unloads_without_rest
          ( fmap<bool>
              ( fhg::util::read_bool
              , optional (node, "require_function_unloads_without_rest")
              )
          );
        const ::boost::optional<bool> require_module_unloads_without_rest
          ( fmap<bool>
              ( fhg::util::read_bool
              , optional (node, "require_module_unloads_without_rest")
              )
          );
        const util::position_type pod_of_eureka_group_attribute
          (state.position (node));
        ::boost::optional<we::type::eureka_id_type> eureka_id
          (optional (node, "eureka-group"));
        bool const got_eureka_attribute (!!eureka_id);
        const util::position_type pod (state.position (node));
        const ::boost::optional<std::string> target
          (optional (node, "target"));
        const std::tuple
          < std::string
          , ::boost::optional<std::string>
          , std::list<std::string>
          , ::boost::optional<std::string>
          , std::list<std::string>
          > sig (parse_function_signature ( signature
                                          , name
                                          , pod
                                          , name
                                          , ports
                                          , memory_buffers
                                          , path
                                          )
                );
        const std::string function (std::get<0> (sig));
        const ::boost::optional<std::string> port_return (std::get<1> (sig));
        const std::list<std::string> port_arg (std::get<2> (sig));
        const ::boost::optional<std::string> memory_buffer_return
          (std::get<3> (sig));
        const std::list<std::string> memory_buffer_arg (std::get<4> (sig));

        ::boost::optional<std::string> code;
        ::boost::optional<util::position_type> pod_of_code;
        std::list<std::string> cincludes;
        std::list<std::string> ldflags;
        if (pass_context.get_value_or (false))
        {
          ldflags.emplace_back ("-ldrts-context");
        }
        std::list<std::string> cxxflags;

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
            else if (child_name == "code")
            {
              pod_of_code = state.position (child);
              code = fhg::util::join (parse_cdata (child, state), '\n').string();
            }
            else if (child_name == "eureka-group")
            {
              std::string const value (child->value(), child->value_size());

              if (got_eureka_attribute)
              {
                throw error::eureka_group_attribute_and_tag
                  ( name
                  , *eureka_id
                  , pod_of_eureka_group_attribute
                  , value
                  , state.position (child)
                  );
              }

              eureka_id = value;
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

        if (is_target_required && !target)
        {
          throw error::missing_target_for_module ( name
                                                 , pod
                                                 );
        }
        else if (!is_target_required && target)
        {
          throw error::modules_without_preferences ( name
                                                   , *target
                                                   , pod
                                                   );
        }

        if (got_eureka_attribute)
        {
          std::ostringstream oss;
          pnet::type::value::value_type const value (*eureka_id);
          oss << pnet::type::value::show (value);
          eureka_id = oss.str();
        }

        return type::module_type
          ( pod
          , name
          , function
          , target
          , port_return
          , port_arg
          , memory_buffer_return
          , memory_buffer_arg
          , code
          , pod_of_code
          , cincludes
          , ldflags
          , cxxflags
          , pass_context
          , eureka_id
          , require_function_unloads_without_rest.get_value_or (true)
          , require_module_unloads_without_rest.get_value_or (true)
          );
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

      type::place_type place_type ( const xml_node_type* node
                                  , state::type& state
                                  , type::function_type::ports_type const& ports
                                  , ::boost::filesystem::path const& path
                                  )
      {
        const std::string name (required ("place_type", node, "name", state));

        type::place_type place
          ( state.position (node)
          , validate_name ( validate_prefix ( name
                                            , "place"
                                            , state.file_in_progress()
                                            )
                          , "place"
                          , state.file_in_progress()
                          )
          , required ("place_type", node, "type", state)
          , fmap<bool>
              (fhg::util::read_bool, optional (node, "virtual"))
          , fmap<bool>
              (fhg::util::read_bool, optional (node, "put_token"))
          );

        if (  place.is_virtual()
           && !ports.has ({place.name(), we::type::port::direction::Tunnel{}})
           )
        {
          state.warn
            (warning::virtual_place_not_tunneled (place.name(), path));
        }

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
              parse_token (child, state, place);
            }
            else if (child_name == "properties")
            {
              property_map_type (place.properties(), child, state);
            }
            else if (child_name == "include-properties")
            {
              const we::type::property::type deeper
                ( properties_include
                  (required ("place_type", child, "href", state), state)
                );

              util::property::join (state, place.properties(), deeper);
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

      type::net_type net_type ( const xml_node_type* node
                              , state::type& state
                              , type::function_type::ports_type const& ports
                              , ::boost::filesystem::path const& path
                              )
      {
        type::net_type::functions_type functions;
        type::net_type::places_type places;
        type::net_type::specializes_type specializes;
        type::net_type::templates_type templates;
        type::net_type::transitions_type transitions;
        type::structs_type structs;
        we::type::property::type properties;

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
              templates.push<error::duplicate_template>
                (tmpl_type (child, state));
            }
            else if (child_name == "specialize")
            {
              specializes.push<error::duplicate_specialize>
                (specialize_type (child, state));
            }
            else if (child_name == "place")
            {
              places.push<error::duplicate_place>
                (place_type (child, state, ports, path));
            }
            else if (child_name == "transition")
            {
              transitions.push<error::duplicate_transition>
                (transition_type (child, state));
            }
            else if (child_name == "struct")
            {
              structs.push_back (struct_type (child, state));
            }
            else if (child_name == "include-structs")
            {
              std::cerr << "TODO: Deprecate and eliminate net::include-structs.\n";
              //! \todo deprecate and eliminate
              const type::structs_type sts
                ( structs_include ( required ("net_type", child, "href", state)
                                  , state
                                  )
                );

              structs.insert (structs.end(), sts.begin(), sts.end());
            }
            else if (child_name == "include-template")
            {
              const std::string file
                (required ("net_type", child, "href", state));

              type::tmpl_type tmpl (template_include (file, state));

              if (not tmpl.name())
              {
                throw error::top_level_anonymous_template (file, "net_type");
              }

              templates.push<error::duplicate_template> (tmpl);
            }
            else if (child_name == "properties")
            {
              property_map_type (properties, child, state);
            }
            else if (child_name == "include-properties")
            {
              const we::type::property::type deeper
                ( properties_include
                    (required ("net_type", child, "href", state), state)
                );

              util::property::join (state, properties, deeper);
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

        return { state.position (node)
               , functions
               , places
               , specializes
               , templates
               , transitions
               , structs
               , properties
               };
      }

      // ******************************************************************* //

      std::tuple<type::multi_module_type, type::preferences_type>
        modules_type ( xml_node_type const* node
                     , state::type& state
                     , type::function_type::ports_type const& ports
                     , fhg::pnet::util::unique<type::memory_buffer_type> const&
                         memory_buffers
                     )
      {
        type::multi_module_type modules;
        ::boost::optional<type::preferences_type> preferences;

        for ( xml_node_type* child (node->first_node())
            ; child
            ; child = child ? child->next_sibling() : child
            )
        {
          const std::string child_name (name_element (child, state));

          if (child)
          {
            if (child_name == "preferences")
            {
              preferences = preferences_type (child, state);
            }
            else if (child_name == "module")
            {
              modules.add ( module_type ( child
                                        , state
                                        , ports
                                        , memory_buffers
                                        , true
                                        , state.position (node).path()
                                        )
                          );
            }
            else
            {
              state.warn ( warning::unexpected_element
                             ( child_name
                             , "modules_type"
                             , state.file_in_progress()
                             )
                         );
            }
          }
        }

        if (modules.modules().empty())
        {
          throw error::preferences_without_modules (state.position (node));
        }
        else if (!preferences)
        {
          throw error::modules_without_preferences (state.position (node));
        }

        return std::make_tuple (std::move (modules), std::move (*preferences));
      }

      type::function_type
        function_type (const xml_node_type* node, state::type& state)
      {
        type::function_type::ports_type ports;
        fhg::pnet::util::unique<type::memory_buffer_type> memory_buffers;
        std::list<type::memory_get> memory_gets;
        std::list<type::memory_put> memory_puts;
        std::list<type::memory_getput> memory_getputs;
        std::list<type::structure_type> structs;
        type::conditions_type conditions;
        type::requirements_type requirements;
        type::preferences_type preferences;
        ::boost::optional<type::expression_type> expression;
        ::boost::optional<type::module_type> module;
        ::boost::optional<type::net_type> net;
        type::multi_module_type multi_module;
        we::type::property::type properties;

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
              ports.push<error::duplicate_port>
                (port_type (child, state, we::type::port::direction::In{}));
            }
            else if (child_name == "out")
            {
              ports.push<error::duplicate_port>
                (port_type (child, state, we::type::port::direction::Out{}));
            }
            else if (child_name == "inout")
            {
              ports.push<error::duplicate_port>
                (port_type (child, state, we::type::port::direction::In{}));
              ports.push<error::duplicate_port>
                (port_type (child, state, we::type::port::direction::Out{}));
            }
            else if (child_name == "tunnel")
            {
              ports.push<error::duplicate_port>
                (port_type (child, state, we::type::port::direction::Tunnel{}));
            }
            else if (child_name == "memory-buffer")
            {
              memory_buffers.push<error::duplicate_memory_buffer>
                (memory_buffer_type (child, state));
            }
            else if (child_name == "memory-get")
            {
              memory_gets.emplace_back (memory_get (child, state));
            }
            else if (child_name == "memory-put")
            {
              memory_puts.emplace_back (memory_put (child, state));
            }
            else if (child_name == "memory-getput")
            {
              memory_getputs.emplace_back (memory_getput (child, state));
            }
            else if (child_name == "struct")
            {
              structs.push_back (struct_type (child, state));
            }
            else if (child_name == "include-structs")
            {
              const type::structs_type sts
                ( structs_include
                  (required ("function_type", child, "href", state), state)
                );

              structs.insert (structs.end(), sts.begin(), sts.end());
            }
            else if (child_name == "expression")
            {
              expression = type::expression_type
                (state.position (child), parse_cdata (child, state));
            }
            else if (child_name == "module")
            {
              module = module_type ( child
                                   , state
                                   , ports
                                   , memory_buffers
                                   , false
                                   , state.position (node).path()
                                   );
            }
            else if (child_name == "modules")
            {
              std::tie (multi_module, preferences)
                = modules_type (child, state, ports, memory_buffers);
            }
            else if (child_name == "net")
            {
              net = net_type
                (child, state, ports, state.position (node).path());
            }
            else if (child_name == "condition")
            {
              auto const cs (parse_cdata (child, state));

              conditions.insert (conditions.end(), cs.begin(), cs.end());
            }
            else if (child_name == "properties")
            {
              property_map_type (properties, child, state);
            }
            else if (child_name == "include-properties")
            {
              const we::type::property::type deeper
                ( properties_include
                  (required ("function_type", child, "href", state), state)
                );

              util::property::join (state, properties, deeper);
            }
            else if (child_name == "require")
            {
              require_type (requirements, child, state);
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

        if (!multi_module.modules().empty())
        {
          is_matching_preferences_and_modules_with_eureka_id ( node
                                                             , state
                                                             , preferences
                                                             , multi_module
                                                             );
        }

#define FUNCTION(_content) type::function_type  \
          { state.position (node)               \
          , optional (node, "name")             \
          , ports                               \
          , memory_buffers                      \
          , memory_gets                         \
          , memory_puts                         \
          , memory_getputs                      \
          , false /* contains_a_module_call */  \
          , structs                             \
          , conditions                          \
          , requirements                        \
          , preferences                         \
          , _content                            \
          , properties                          \
          }

        if (!!expression)
        {
          return FUNCTION (*expression);
        }

        if (!!multi_module.modules().size())
        {
          return FUNCTION (multi_module);
        }

        if (!!module)
        {
          return FUNCTION (*module);
        }

        if (!!net)
        {
          return FUNCTION (*net);
        }

        throw std::logic_error ("missing function content");

#undef FUNCTION
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
        std::unordered_set<std::string> names;
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
              std::string const name
                (required ("struct_field", child, "name", state));

              if (!names.emplace (name).second)
              {
                throw error::struct_field_redefined
                  (name, state.file_in_progress());
              }

              s.push_back ( std::make_pair
                            ( name
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
          ( state.position (node)
          , structured_type (node, state)
          );
      }
    }

    // ********************************************************************* //

    type::function_type just_parse (state::type& state, std::istream& stream)
    {
      return state.generic_parse<type::function_type>
        ( std::bind ( generic_parse<type::function_type>
                    , function_type, std::placeholders::_1, std::placeholders::_2
                    , "defun", "parse_function"
                    )
        , stream
        );
    }

    type::function_type just_parse
      (state::type& state, ::boost::filesystem::path const& input)
    {
      state.set_input (input);

      return state.generic_parse<type::function_type>
        ( std::bind ( generic_parse<type::function_type>
                    , function_type, std::placeholders::_1, std::placeholders::_2
                    , "defun", "parse_function"
                    )
        , input
        );
    }

    void post_processing_passes ( type::function_type& function
                                , state::type* state
                                )
    {
      // set all the collected requirements to the top level function
      function.requirements = state->requirements();

      function.specialize (*state);

      {
        //! \note type check needs resolved functions to detect type of
        //! port types in function inside using-transition
        std::unordered_map<std::string, type::function_type const&> known;
        function.resolve_function_use_recursive (known);
      }

      {
        std::unordered_map<std::string, pnet::type::signature::signature_type> known;
        function.resolve_types_recursive (known);
      }

      function.type_check (*state);
    }

    void generate_cpp ( type::function_type const& function_in
                      , state::type const& state
                      )
    {
      type::fun_info_map m;

      type::function_type function (function_in);
      type::find_module_calls (state, function, m);

      type::mk_wrapper (state, m);

      std::unordered_set<std::string> structnames;

      type::struct_to_cpp (state, function, structnames);

      type::mk_makefile (state, m, structnames);
    }

    void dump_xml ( type::function_type const& function
                  , state::type const& state
                  )
    {
      std::string const& file (state.dump_xml_file());

      std::ofstream stream (file.c_str());
      if (!stream)
      {
        throw error::could_not_open_file (file);
      }

      fhg::util::xml::xmlstream s (stream);

      type::dump::dump (s, function);
    }

    we::type::Transition xml_to_we
      ( xml::parse::type::function_type const& function
      , xml::parse::state::type const& state
      )
    {
      if (not function.name())
      {
        state.warn (warning::synthesize_anonymous_function (function));
      }

      std::unordered_map<std::string, we::port_id_type> port_id_in;
      std::unordered_map<std::string, we::port_id_type> port_id_out;
      std::unordered_map<we::port_id_type, std::string> real_place_names;

      return function.synthesize
          ( function.name().get_value_or ("anonymous")
          , state
          , port_id_in
          , port_id_out
          , type::conditions_type()
          , we::type::property::type()
          , type::requirements_type()
          , we::priority_type()
          , {}
          , real_place_names
          )
        ;
    }
  }
}
