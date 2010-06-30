
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
    static void struct_type (const xml_node_type *, state::type &);
    static type::token token_type (const xml_node_type *, state::type &);
    static type::transition transition_type (const xml_node_type *, state::type &);

    static void parse (std::istream & f, state::type &);

    // ********************************************************************* //

    static std::string
    name_element (xml_node_type * & node, state::type & state)
    {
      skip (node, rapidxml::node_comment);
      expect (node, rapidxml::node_element);

      const std::string name (node->name());

      if (name == "xi:include")
        {
          std::string file ("example/kdm/");
          file.append (node->first_attribute("href")->value());

          cout << "*** include START " << file << endl;

          std::ifstream f (file.c_str());

          parse (f, state);

          cout << "*** include END " << file << endl;

          node = node->next_sibling();

          return name_element (node, state);
        }

      return name;
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
          const std::string child_name (name_element (child, state));

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
              struct_type (child, state);
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
          const std::string child_name (name_element (child, state));

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
          const std::string child_name (name_element (child, state));

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

    static void
    struct_type (const xml_node_type * node, state::type & state)
    {
      const std::string name (required ("struct_type", node, "name"));

      if (state.signature().find (name) != state.signature().end())
        {
          throw exception::error
            ("struct_type", "struct already defined " + util::quote(name));
        }

      state.signature()[name] = signature::structured_t();

      gen_struct_type (node, state, state.signature()[name]);
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
          const std::string child_name (name_element (child, state));

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
          const std::string child_name (name_element (child, state));

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
      t.use = optional (node, "use");

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (name_element (child, state));

          if (child_name == "defun")
            {
              if (t.use.isJust())
                {
                  throw exception::error 
                    ("transition_type", "use and defun given at the same time");
                }

              ++state.level();

              t.f = Just<>(function_type (child, state));

              --state.level();
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
              throw exception::unexpected_element ("transition_type", child_name);
            }
        }

      return t;
    }

    // ********************************************************************* //

    static int level (0);

    static void
    parse (std::istream & f, state::type & state)
    {
      ++state.level();

      xml_document_type doc;

      input_type inp (f);

      doc.parse < rapidxml::parse_full
                | rapidxml::parse_trim_whitespace
                | rapidxml::parse_normalize_whitespace
                > (inp.data())
        ;

      for ( xml_node_type * node (doc.first_node())
          ; node
          ; node = node->next_sibling()
          )
        {
          skip (node, rapidxml::node_declaration);

          const std::string name (name_element (node, state));
      
          if (name == "defun")
            {
              type::function F (function_type (node, state));

              if (state.level() == 1)
                {
                  cout << F;
                }
            }
          else
            {
              throw exception::unexpected_element ("parse", name);
            }
        }

      --state.level();
    }
  }
}

// ************************************************************************* //

int
main (void)
{
  xml::parse::state::type state;

  xml::parse::parse (std::cin, state);

  //  std::ifstream f ("/u/r/rahn/SDPA/trunk/xml/example/kdm/simple_kdm.xml");
  //  xml::parse::parse (f);

  std::cout << std::endl << "--- parsing DONE" << std::endl;

  state.print_signatures(std::cout);

  std::cout << std::endl << "...resolved signatures" << std::endl;

  state.resolve_signatures();

  state.print_signatures(std::cout);

  return EXIT_SUCCESS;
}
