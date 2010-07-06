
#include <parse/rapidxml/1.13/rapidxml.hpp>
#include <parse/rapidxml/1.13/rapidxml_utils.hpp>

#include <parse/util.hpp>

#include <parse/error.hpp>
#include <parse/warning.hpp>
#include <parse/types.hpp>
#include <parse/state.hpp>

#include <we/type/signature.hpp>
#include <we/type/id.hpp>

#include <we/util/read.hpp>

#include <iostream>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

// ************************************************************************* //

namespace xml
{
  namespace parse
  {
    // ********************************************************************* //

    static type::connect connect_type (const xml_node_type *, state::type &);
    static type::function function_type (const xml_node_type *, state::type &);
    static type::mod mod_type (const xml_node_type *, state::type &);
    static type::net net_type (const xml_node_type *, state::type &);
    static type::place place_type (const xml_node_type *, state::type &);
    static type::port port_type (const xml_node_type *, state::type &);
    static void gen_struct_type ( const xml_node_type *, state::type &
                                , signature::desc_t &
                                );
    static void substruct_type ( const xml_node_type *, state::type &
                               , signature::desc_t &
                               );
    static type::struct_t struct_type (const xml_node_type *, state::type &);
    static type::token token_type (const xml_node_type *, state::type &);
    static type::transition transition_type ( const xml_node_type *
                                            , state::type &
                                            );

    static type::function parse_function (std::istream & f, state::type &);


    typedef std::vector<type::struct_t> struct_vec_type;

    static struct_vec_type structs_include ( const std::string &
                                           , state::type &
                                           );
    static struct_vec_type parse_structs (std::istream &, state::type &);
    static struct_vec_type structs_type ( const xml_node_type *
                                        , state::type & state
                                        );

    // ********************************************************************* //

    static type::function
    function_include (const std::string & file, state::type & state)
    {
      return state.generic_include<type::function> (parse_function, file);
    }

    static struct_vec_type
    structs_include (const std::string & file, state::type & state)
    {
      return state.generic_include<struct_vec_type> (parse_structs, file);
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
          throw error::no_elements_given (pre);
        }

      skip (node, rapidxml::node_declaration);

      const std::string name (name_element (node));

      if (name != name_wanted)
        {
          throw error::unexpected_element (name, pre);
        }

      if (node->next_sibling())
        {
          throw error::more_than_one_definition (pre);
        }

      return parse (node, state);
    };

    static type::function
    parse_function (std::istream & f, state::type & state)
    {
      return generic_parse (function_type, f, state, "defun", "parse_function");
    }

    static struct_vec_type
    parse_structs (std::istream & f, state::type & state)
    {
      return generic_parse (structs_type, f, state, "structs", "parse_structs");
    }

    // ********************************************************************* //

    static struct_vec_type
    structs_type (const xml_node_type * node, state::type & state)
    {
      struct_vec_type v;

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (child->name());

          if (child_name == "struct")
            {
              v.push_back (struct_type (child, state));
            }
          else if (child_name == "include-structs")
            {
              const struct_vec_type struct_vec 
                (structs_include (required ( "structs_type"
                                           , child
                                           , "href"
                                           )
                                 , state
                                 )
                );

              v.insert (v.end(), struct_vec.begin(), struct_vec.end());
            }
          else
            {
              throw error::unexpected_element (child_name, "structs_type");
            }
        }

      return v;
    }

    // ********************************************************************* //

    static type::connect
    connect_type (const xml_node_type * node, state::type &)
    {
      return type::connect ( required ("connect_type", node, "place")
                           , required ("connect_type", node, "port")
                           );
    }

    // ********************************************************************* //

    static type::function
    function_type (const xml_node_type * node, state::type & state)
    {
      type::function f;

      f.level = state.level();
      f.name = optional (node, "name");
      f.internal = fmap<std::string, bool>( read_bool
                                          , optional (node, "internal")
                                          );

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (child->name());

          if (child_name == "in")
            {
              f.in.push_back (port_type (child, state));
            }
          else if (child_name == "out")
            {
              f.out.push_back (port_type (child, state));
            }
          else if (child_name == "struct")
            {
              f.structs.push_back (struct_type (child, state));
            }
          else if (child_name == "include-structs")
            {
              const struct_vec_type struct_vec 
                (structs_include (required ( "function_type"
                                           , child
                                           , "href"
                                           )
                                 , state
                                 )
                );

              f.structs.insert ( f.structs.end()
                               , struct_vec.begin()
                               , struct_vec.end()
                               );
            }
          else if (child_name == "expression")
            {
              f.f = type::expression (child->value());
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
              f.cond.push_back (std::string (child->value()));
            }
          else
            {
              throw error::unexpected_element (child_name, "function_type");
            }
        }

      return f;
    }

    // ********************************************************************* //

    static type::mod
    mod_type (const xml_node_type * node, state::type &)
    {
      return type::mod ( required ("mod_type", node, "name")
                       , required ("mod_type", node, "function")
                       );
    }

    // ********************************************************************* //

    static type::net
    net_type (const xml_node_type * node, state::type & state)
    {
      type::net n;

      n.level = state.level();

      ++state.level();

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (name_element (child));

          if (child_name == "defun")
            {
              n.element.push_back (function_type (child, state));
            }
          else if (child_name == "place")
            {
              n.element.push_back (place_type (child, state));
            }
          else if (child_name == "transition")
            {
              n.element.push_back (transition_type (child, state));
            }
          else if (child_name == "struct")
            {
              n.element.push_back (struct_type (child, state));
            }
          else if (child_name == "include-structs")
            {
              const struct_vec_type struct_vec 
                (structs_include (required ("net_type", child, "href"), state));

              n.element.insert ( n.element.end()
                               , struct_vec.begin()
                               , struct_vec.end()
                               );
            }
          else if (child_name == "include")
            {
              const std::string file (required ("net_type", child, "href"));
              const maybe<std::string> as (optional (child, "as"));

              type::function fun (function_include (file, state));

              if (as.isJust())
                {
                  if (fun.name.isJust())
                    {
                      state.warn (warning::overwrite_function_name (*(fun.name)
                                                                   , *as
                                                                   )
                                 );
                    }

                  fun.name = *as;
                }

              if (fun.name.isNothing())
                {
                  throw error::top_level_anonymous_function (file, "net_type");
                }

              n.element.push_back (fun);
            }
          else
            {
              throw error::unexpected_element (child_name, "net_type");
            }
        }

      --state.level();

      return n;
    }

    // ********************************************************************* //

    static type::place
    place_type (const xml_node_type * node, state::type & state)
    {
      type::place p
        ( required ("place_type", node, "name")
        , required ("place_type", node, "type")
        , fmap<std::string, petri_net::capacity_t>
          ( &::we::util::reader<petri_net::capacity_t>::read
          , optional (node, "capacity")
          )
        );

      p.level = state.level();

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (name_element (child));

          if (child_name == "token")
            {
              p.push_token (token_type (child, state));
            }
          else
            {
              throw error::unexpected_element (child_name, "place_type");
            }
        }

      return p;
    }

    // ********************************************************************* //

    static type::port
    port_type (const xml_node_type * node, state::type &)
    {
      return type::port ( required ("port_type", node, "name")
                        , required ("port_type", node, "type")
                        , optional (node, "place")
                        );
    }

    // ********************************************************************* //

    static void
    struct_field_type ( const xml_node_type * node
                      , state::type &
                      , signature::desc_t & sig
                      )
    {
      const std::string name (required ("struct_field_type", node, "name"));
      const std::string type (required ("struct_field_type", node, "type"));

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
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (name_element (child));

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
              throw error::unexpected_element (child_name, "gen_struct_type");
            }
        }
    }

    static void
    substruct_type ( const xml_node_type * node
                   , state::type & state
                   , signature::desc_t & sig
                   )
    {
      const std::string name (required ("substruct_type", node, "name"));

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

      s.name = required ("struct_type", node, "name");
      s.sig = signature::structured_t();
      s.level = state.level();

      gen_struct_type (node, state, s.sig);

      return s;
    }

    // ********************************************************************* //

    static void
    token_field_type ( const xml_node_type * node
                     , state::type & state
                     , type::token & tok
                     )
    {
      const std::string name (required ("token_field_type", node, "name"));
      
      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (name_element (child));

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
                  ( signature::visitor::get_or_create_structured_field ( name
                                                                       , "token"
                                                                       )
                  , tok
                  )
                );
            }
          else
            {
              throw error::unexpected_element (child_name, "token_field_type");
            }
        }
    }

    // ********************************************************************* //

    static type::token
    token_type (const xml_node_type * node, state::type & state)
    {
      type::token tok = signature::structured_t();

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (name_element (child));

          if (child_name == "value")
            {
              return type::token (std::string (child->value()));
            }
          else if (child_name == "field")
            {
              token_field_type (child, state, tok);
            }
          else
            {
              throw error::unexpected_element (child_name, "token_type");
            }
        }

      return tok;
    }

    // ********************************************************************* //

    static type::transition
    transition_type (const xml_node_type * node, state::type & state)
    {
      type::transition t;

      t.level = state.level();
      t.name = required ("transition_type", node, "name");

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (name_element (child));

          if (child_name == "include")
            {
              const std::string file (required ( "transition_type"
                                               , child
                                               , "href"
                                               )
                                     );

              state.level() += 2;;

              t.f = function_include (file, state);

              state.level() -= 2;
            }
          else if (child_name == "use")
            {
              t.f = type::use ( required ("transition_type", child, "name")
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
              t.in.push_back (connect_type(child, state));
            }
          else if (child_name == "connect-out")
            {
              t.out.push_back (connect_type(child, state));
            }
          else if (child_name == "connect-read")        
            {
              t.read.push_back (connect_type(child, state));
            }
          else
            {
              throw error::unexpected_element (child_name, "transition_type");
            }
        }

      return t;
    }
  }
}

// ************************************************************************* //

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  xml::parse::state::type state;
  std::string input;

  po::options_description desc("options");

  desc.add_options()
    ("help", "this message")
    ( "search-path"
    , po::value<xml::parse::state::search_path_type>(&state.search_path())
    , "search path"
    )
    ( "input"
    , po::value<std::string>(&input)->default_value("-")
    , "input file name, - for stdin"
    )
    ( "Werror"
    , po::value<bool>(&state.Werror())->default_value(false)
    , "cast warnings to errors"
    )
    ( "Woverwrite_function_name"
    , po::value<bool>(&state.Woverwrite_function_name())->default_value(true)
    , "warn when overwriting a function name"
    )
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
    }

  const xml::parse::type::function f
    ((input == "-") 
    ? xml::parse::parse_function (std::cin, state)
    : xml::parse::function_include (input, state)
    );

  std::cout << f << std::endl;
  
  return EXIT_SUCCESS;
}
