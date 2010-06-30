
#include <parse/rapidxml/1.13/rapidxml.hpp>
#include <parse/rapidxml/1.13/rapidxml_utils.hpp>

#include <parse/util/maybe.hpp>
#include <parse/util/show_node_type.hpp>

#include <parse/exception.hpp>
#include <parse/types.hpp>

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

    namespace state
    {
      struct type
      {
      private:
        signature::set_type _signature;

      public:
        type (void)
          : _signature ()
        {}

        signature::set_type & signature (void) { return _signature; }

        void resolve_signatures (void)
        {
          signature::visitor::resolve resolve (_signature);

          for ( signature::set_type::iterator sig (_signature.begin())
              ; sig != _signature.end()
              ; ++sig
              )
            {
              boost::apply_visitor (resolve, sig->second);
            }
        }

        void print_signatures (void) const
        {
          cout << "signatures:" << endl;

          for ( signature::set_type::const_iterator pos (_signature.begin())
              ; pos != _signature.end()
              ; ++pos
              )
            {
              cout << pos->first << ": " << pos->second << endl;
            }
        }
      };
    }

    // ********************************************************************* //

    typedef char Ch;
    typedef rapidxml::xml_node<Ch> xml_node_type;
    typedef rapidxml::xml_document<Ch> xml_document_type;
    typedef rapidxml::file<Ch> input_type;

    // ********************************************************************* //

    static void connect_type (const xml_node_type *, state::type &);
    static void function_type (const xml_node_type *, state::type &);
    static void mod_type (const xml_node_type *, state::type &);
    static void net_type (const xml_node_type *, state::type &);
    static void place_type (const xml_node_type *, state::type &);
    static void port_type (const xml_node_type *, state::type &);
    static void gen_struct_type (const xml_node_type *, state::type &, signature::desc_t &);
    static void substruct_type (const xml_node_type *, state::type &, signature::desc_t &);
    static void struct_type (const xml_node_type *, state::type &);
    static type::token token_type (const xml_node_type *, state::type &);
    static void transition_type (const xml_node_type *, state::type &);

    static void parse (std::istream & f, state::type &);

    // ********************************************************************* //

    static void
    skip (xml_node_type * & node, const rapidxml::node_type t)
    {
      while (node && (node->type() == t))
        {
          node = node->next_sibling();
        }
    }

    static void
    expect (xml_node_type * & node, const rapidxml::node_type t)
    {
      skip (node, rapidxml::node_comment);

      if (!node)
        {
          throw exception::node_type (t);
        }

      if (node->type() != rapidxml::node_element)
        {
          throw exception::node_type (t, node->type());
        }
    }

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

    static std::string
    required ( const std::string & pre
             , const xml_node_type * node
             , const Ch * attr
             )
    {
      if (!node->first_attribute (attr))
        {
          throw exception::missing_attr (pre, attr);
        }

      return node->first_attribute (attr)->value();
    }

    static std::string
    optional ( const xml_node_type * node
             , const Ch * attr
             , const std::string & dflt
             )
    {
      return node->first_attribute (attr) 
        ? node->first_attribute (attr)->value() 
        : dflt
        ;
    }

    static bool
    optional (const xml_node_type * node, const Ch * attr, std::string & val)
    {
      if (node->first_attribute (attr))
        {
          val = std::string (node->first_attribute (attr)->value());
        }

      return node->first_attribute (attr);
    }

    static maybe<std::string>
    optional (const xml_node_type * node, const Ch * attr)
    {
      return node->first_attribute (attr) 
        ? Just<>(std::string(node->first_attribute (attr)->value()))
        : Nothing<std::string>()
        ;
    }

    // ********************************************************************* //

    static void
    connect_type (const xml_node_type * node, state::type &)
    {
      const type::connect c
        ( required ("connect_type", node, "place")
        , required ("connect_type", node, "port")
        );

      cout << c << endl;
    }

    // ********************************************************************* //

    static void
    function_type (const xml_node_type * node, state::type & state)
    {
      const std::string name (optional (node, "name", "[anonymous]"));
      const std::string internal (optional (node, "internal", "true"));

      cout << "name: " << name << ", internal: " << internal << endl;

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (child->name());

          if (child_name == "in")
            {
              cout << "in: "; port_type (child, state);
            }
          else if (child_name == "out")
            {
              cout << "out: "; port_type (child, state);
            }
          else if (child_name == "expression")
            {
              cout << "expression: " << child->value() << endl;
            }
          else if (child_name == "module")
            {
              mod_type (child, state);
            }
          else if (child_name == "net")
            {
              cout << "net: " << endl; net_type (child, state);
            }
          else if (child_name == "condition")
            {
              cout << "condition: " << child->value() << endl;
            }
          else
            {
              throw exception::unexpected_element ("function_type", child_name);
            }
        }
    }

    // ********************************************************************* //

    static void
    mod_type (const xml_node_type * node, state::type &)
    {
      const type::mod m
        ( required ("mod_type", node, "name")
        , required ("mod_type", node, "function")
        );

      cout << m << endl;
    }

    // ********************************************************************* //

    static void
    net_type (const xml_node_type * node, state::type & state)
    {
      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (name_element (child, state));

          if (child_name == "defun")
            {
              cout << "defun: "; function_type (child, state);
            }
          else if (child_name == "place")
            {
              place_type (child, state);
            }
          else if (child_name == "transition")
            {
              cout << "transition: "; transition_type (child, state);
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
    }

    // ********************************************************************* //

    static void
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

      cout << p << endl;
    }

    // ********************************************************************* //

    static void
    port_type (const xml_node_type * node, state::type &)
    {
      const type::port p
        ( required ("port_type", node, "name")
        , required ("port_type", node, "type")
        , optional (node, "place")
        );

      cout << p << endl;
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

    static void
    transition_type (const xml_node_type * node, state::type & state)
    {
      const std::string name (required ("transition_type", node, "name"));

      cout << "name: " << name;

      std::string use;
      const bool got_use (optional (node, "use", use));

      if (got_use)
        {
          cout << ", use: " << use;
        }

      cout << endl;

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string child_name (name_element (child, state));

          if (child_name == "defun")
            {
              if (got_use)
                {
                  throw exception::error 
                    ("transition_type", "use and defun given at the same time");
                }

              cout << "defun: "; function_type (child, state);
            }
          else if (child_name == "connect-in")
            {
              cout << "connect_in: "; connect_type(child, state);
            }
          else if (child_name == "connect-out")
            {
              cout << "connect_out: "; connect_type(child, state);
            }
          else if (child_name == "connect-read")        
            {
              cout << "connect_read: "; connect_type(child, state);
            }
          else
            {
              throw exception::unexpected_element ("transition_type", child_name);
            }
        }
    }

    // ********************************************************************* //

    static void
    parse (std::istream & f, state::type & state)
    {
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
              cout << "defun: "; function_type (node, state);
            }
          else
            {
              throw exception::unexpected_element ("parse", name);
            }
        }
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

  state.print_signatures();

  std::cout << std::endl << "...resolved signatures" << std::endl;

  state.resolve_signatures();

  state.print_signatures();

  return EXIT_SUCCESS;
}
