// mirko.rahn@itwm.fraunhofer.de

#ifndef _XML_PARSE_PARSER_HPP
#define _XML_PARSE_PARSER_HPP

#include <xml/parse/rapidxml/types.hpp>

#include <xml/parse/util.hpp>

#include <xml/parse/id/types.fwd.hpp>

#include <xml/parse/type/connect.fwd.hpp>
#include <xml/parse/type/expression.fwd.hpp>
#include <xml/parse/type/function.fwd.hpp>
#include <xml/parse/type/mod.fwd.hpp>
#include <xml/parse/type/net.fwd.hpp>
#include <xml/parse/type/place.fwd.hpp>
#include <xml/parse/type/place_map.fwd.hpp>
#include <xml/parse/type/port.fwd.hpp>
#include <xml/parse/type/specialize.fwd.hpp>
#include <xml/parse/type/struct.fwd.hpp>
#include <xml/parse/type/template.fwd.hpp>
#include <xml/parse/type/token.fwd.hpp>
#include <xml/parse/type/transition.fwd.hpp>
#include <xml/parse/type/use.fwd.hpp>

#include <xml/parse/type/function.hpp>
#include <xml/parse/type/net.hpp>

#include <xml/parse/error.hpp>
#include <xml/parse/warning.hpp>
#include <xml/parse/state.hpp>

#include <we/type/signature.hpp>
#include <we/type/id.hpp>
#include <we/type/property.hpp>

#include <boost/function.hpp>

#include <sstream>

// ************************************************************************* //

namespace xml
{
  namespace parse
  {
    type::connect_type connect_type ( const xml_node_type *
                                    , state::type &
                                    , const id::transition& parent
                                    );
    id::function function_type ( const xml_node_type *
                               , state::type &
                               , const boost::optional<type::function_type::id_parent>& parent
                               );
    type::module_type module_type ( const xml_node_type *
                            , state::type &
                            , const id::function& parent
                            );
    id::tmpl tmpl_type ( const xml_node_type *
                       , state::type &
                       , const id::net& parent
                       );
    id::net net_type ( const xml_node_type *
                     , state::type &
                     , const id::function& parent
                     );
    id::place place_type ( const xml_node_type *
                         , state::type &
                         , const id::net& parent
                         );
    type::port_type port_type ( const xml_node_type *
                              , state::type &
                              , const id::function& parent
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
    type::token_type token_type ( const xml_node_type *
                                , state::type &
                                , const id::place& parent
                                );
    id::transition transition_type ( const xml_node_type *
                                   , state::type &
                                   , const id::net& parent
                                   );
    id::specialize specialize_type ( const xml_node_type *
                                   , state::type &
                                   , const id::net& parent
                                   );
    int property_map_type ( we::type::property::type &
                          , const xml_node_type *
                          , state::type &
                          );
    we::type::property::type
    property_maps_type (const xml_node_type *, state::type &);

    type::structs_type structs_type ( const xml_node_type *
                                    , state::type & state
                                    , const id::function& parent
                                    );

    id::function
    just_parse (state::type & state, const std::string & input);

    // ********************************************************************* //

    template<typename T>
    static T
    generic_parse ( boost::function<T (const xml_node_type *, state::type &)> parse
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
  }
}

#endif
