
#include <parse/rapidxml/1.13/rapidxml.hpp>
#include <parse/rapidxml/1.13/rapidxml_utils.hpp>

#include <parse/util.hpp>

#include <parse/exception.hpp>
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

    using std::cout;
    using std::endl;

    // ********************************************************************* //

    static type::connect connect_type (const xml_node_type *, state::type &);
    static type::function function_type (const xml_node_type *, state::type &);
    static type::mod mod_type (const xml_node_type *, state::type &);
    static type::net net_type (const xml_node_type *, state::type &);
    static type::place place_type (const xml_node_type *, state::type &);
    static type::port port_type (const xml_node_type *, state::type &);
    static void gen_struct_type (const xml_node_type *, state::type &, signature::desc_t &);
    static void substruct_type (const xml_node_type *, state::type &, signature::desc_t &);
    static type::struct_t struct_type (const xml_node_type *, state::type &);
    static type::token token_type (const xml_node_type *, state::type &);
    static type::transition transition_type (const xml_node_type *, state::type &);

    static type::function parse (std::istream & f, state::type &);

    // ********************************************************************* //

    static std::string
    name_element (xml_node_type * & node)
    {
      skip (node, rapidxml::node_comment);
      expect (node, rapidxml::node_element);

      return node->name();
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

    static bool
    read_bool (const std::string & inp)
    {
      if (inp == "true")
        {
          return true;
        }
      else if (inp == "false")
        {
          return false;
        }
      else
        {
          throw std::runtime_error ("failed to read a bool from: " + inp);
        }
    }

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
              throw exception::unexpected_element ("function_type", child_name);
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
          else
            {
              throw exception::unexpected_element ("net_type", child_name);
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
              throw exception::unexpected_element ("place_type", child_name);
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
              throw exception::unexpected_element
                ("gen_struct_type", child_name);
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
              throw exception::unexpected_element
                ("token_field_type", child_name);
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
              throw exception::unexpected_element ("token_type", child_name);
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
              namespace fs = boost::filesystem;

              const std::string file (required ( "transition_type"
                                               , child
                                               , "name"
                                               )
                                     );
              const fs::path path (state.expand (file));

              cout << "*** include START " << path << endl;

              std::ifstream f (path.string().c_str());

              state.level() += 2;

              t.f = parse (f, state);

              state.level() -= 2;

              cout << "*** include END " << path << endl;
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
              throw exception::unexpected_element ( "transition_type"
                                                  , child_name
                                                  );
            }
        }

      return t;
    }

    // ********************************************************************* //

    static type::function
    parse (std::istream & f, state::type & state)
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
          throw exception::error ("parse", "no element given at all!?");
        }

      skip (node, rapidxml::node_declaration);

      const std::string name (name_element (node));

      if (name != "defun")
        {
          throw exception::unexpected_element ("parse", name);
        }

      type::function fun (function_type (node, state));

      if (node->next_sibling())
        {
          throw exception::error
            ("parse", "more than one function definition in one file");
        }

      return fun;
    }
  }
}

// ************************************************************************* //

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  xml::parse::state::type state;

  po::options_description desc("options");

  desc.add_options()
    ("help", "this message")
    ( "search-path"
    , po::value<xml::parse::state::search_path_type>(&state.search_path())
    , "search path"
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

  //  std::cout << xml::parse::parse (std::cin, state);
  
  std::ifstream f ("example/kdm/simple_kdm.xml");
  std::cout << xml::parse::parse (f, state);

  std::cout << std::endl << "--- parsing DONE" << std::endl;

  return EXIT_SUCCESS;
}
