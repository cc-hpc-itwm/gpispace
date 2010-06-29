
#include "rapidxml/1.13/rapidxml.hpp"
#include "rapidxml/1.13/rapidxml_utils.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>

// ************************************************************************* //

namespace xml
{
  namespace parse
  {
    namespace util
    {
      static std::string
      show_node_type (const int t)
      {
        switch (t)
          {
          case rapidxml::node_document: return "document";
          case rapidxml::node_element: return "element";
          case rapidxml::node_data: return "data";
          case rapidxml::node_cdata: return "cdata";
          case rapidxml::node_comment: return "comment";
          case rapidxml::node_declaration: return "declaration";
          case rapidxml::node_doctype: return "doctype";
          case rapidxml::node_pi: return "pi";
          default: throw std::runtime_error ("STRANGE: unknown node type");
          }
      }

      static std::string
      quote (const std::string & str)
      {
        return "\"" + str + "\"";
      }
    }

    namespace exception
    {
      class unexpected_element : public std::runtime_error
      {
      public:
        unexpected_element ( const std::string & pre
                           , const std::string & name
                           )
          : std::runtime_error 
            (pre + ": unexpected element with name" + util::quote(name))
        {}
      };

      class node_type : public std::runtime_error
      {
      public:
        node_type (const rapidxml::node_type & want)
          : std::runtime_error ("missing node: expexted node of type "
                               + util::quote(util::show_node_type (want))
                               )
        {}
        node_type ( const rapidxml::node_type & want
                  , const rapidxml::node_type & got
                  )
          : std::runtime_error ("wrong node type: expexted node of type "
                               + util::quote(util::show_node_type (want))
                               + ": got node of type "
                               + util::quote(util::show_node_type (got))
                               )
        {}
      };

      class missing_attr : public std::runtime_error
      {
      public:
        missing_attr ( const std::string & pre
                     , const std::string & attr
                     )
          : std::runtime_error
            (pre + ": missing attribute " + util::quote(attr))
        {}
      };

      class error : public std::runtime_error
      {
      public:
        error (const std::string & pre, const std::string & what)
          : std::runtime_error (pre + ": " + what)
        {}
      };
    }

    // ********************************************************************* //

    using std::cout;
    using std::endl;

    // ********************************************************************* //

    typedef char Ch;
    typedef rapidxml::xml_node<Ch> xml_node_type;
    typedef rapidxml::xml_document<Ch> xml_document_type;
    typedef rapidxml::file<Ch> input_type;

    // ********************************************************************* //

    static void connect_type (const xml_node_type *);
    static void function_type (const xml_node_type *);
    static void mod_type (const xml_node_type *);
    static void net_type (const xml_node_type *);
    static void place_type (const xml_node_type *);
    static void port_type (const xml_node_type *);
    static void struct_field_type (const xml_node_type *);
    static void struct_type (const xml_node_type *);
    static void token_field_type (const xml_node_type *);
    static void token_type (const xml_node_type *);
    static void transition_type (const xml_node_type *);

    static void parse (std::istream & f);

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
    name_element (xml_node_type * & node)
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

          parse (f);

          cout << "*** include END " << file << endl;

          node = node->next_sibling();

          return name_element (node);
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

    // ********************************************************************* //

    static void
    connect_type (const xml_node_type * node)
    {
      const std::string place (required ("connect_type", node, "place"));
      const std::string port (required ("connect_type", node, "port"));

      cout << "place: " << place << ", port: " << port << endl;
    }

    static void
    function_type (const xml_node_type * node)
    {
      const std::string name (optional (node, "name", "[anonymous]"));
      const std::string internal (optional (node, "internal", "true"));

      cout << "name: " << name << ", internal: " << internal << endl;

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string name (child->name());

          if (name == "in")
            {
              cout << "in: "; port_type (child);
            }
          else if (name == "out")
            {
              cout << "out: "; port_type (child);
            }
          else if (name == "expression")
            {
              cout << "expression: " << child->value() << endl;
            }
          else if (name == "module")
            {
              cout << "module: "; mod_type (child);
            }
          else if (name == "net")
            {
              cout << "net: " << endl; net_type (child);
            }
          else if (name == "condition")
            {
              cout << "condition: " << child->value() << endl;
            }
          else
            {
              throw exception::unexpected_element ("function_type", name);
            }
        }
    }

    static void
    mod_type (const xml_node_type * node)
    {
      const std::string mod (required ("mod_type", node, "name"));
      const std::string fun (required ("mod_type", node, "function"));

      cout << "mod: " << mod << ", fun " << fun << endl;
    }

    static void
    net_type (const xml_node_type * node)
    {
      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string name (name_element (child));

          if (name == "defun")
            {
              cout << "defun: "; function_type (child);
            }
          else if (name == "place")
            {
              cout << "place: "; place_type (child);
            }
          else if (name == "transition")
            {
              cout << "transition: "; transition_type (child);
            }
          else if (name == "struct")
            {
              cout << "struct: "; struct_type (child);
            }
          else
            {
              throw exception::unexpected_element ("net_type", name);
            }
        }
    }

    static void
    place_type (const xml_node_type * node)
    {
      const std::string name (required ("place_type", node, "name"));
      const std::string type (required ("place_type", node, "type"));

      cout << " name: " << name << ", type: " << type << endl;

      std::string capacity;

      if (optional (node, "capacity", capacity))
        {
          cout << ", capacity: " << capacity;
        }

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string name (name_element (child));

          if (name == "token")
            {
              cout << "token: "; token_type (child);
            }
          else
            {
              throw exception::unexpected_element ("place_type", name);
            }
        }
    }

    static void
    port_type (const xml_node_type * node)
    {
      const std::string name (required ("port_type", node, "name"));
      const std::string type (required ("port_type", node, "type"));

      cout << name << " :: " << type;

      std::string connected_to;

      if (optional (node, "place", connected_to))
        {
          cout << ", connected to " << connected_to;
        }

      cout << endl;
    }

    static void
    struct_field_type (const xml_node_type * node)
    {
      const std::string name (required ("struct_field_type", node, "name"));
      const std::string type (required ("struct_field_type", node, "type"));

      cout << "name: " << name << ", type: " << type << endl;
    }

    static void
    struct_type (const xml_node_type * node)
    {
      const std::string name (required ("struct_type", node, "name"));

      cout << "name: " << name << endl;

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string name (name_element (child));

          if (name == "field")
            {
              cout << "field: "; struct_field_type (child);
            }
          else if (name == "struct")
            {
              cout << "struct: "; struct_type (child);
            }
          else
            {
              throw exception::unexpected_element ("struct_type", name);
            }
        }
    }

    static void
    token_field_type (const xml_node_type * node)
    {
      const std::string name (required ("token_field_type", node, "name"));
      
      cout << "name: " << name << endl;

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string name (name_element (child));

          if (name == "value")
            {
            }
          else if (name == "field")
            {
              cout << "field: "; token_field_type (child);
            }
          else
            {
              throw exception::unexpected_element ("token_field_type", name);
            }
        }
    }

    static void
    token_type (const xml_node_type * node)
    {
      const std::string type (required ("token_type", node, "type"));

      cout << "type: " << type << endl;

      for ( xml_node_type * child (node->first_node())
          ; child
          ; child = child->next_sibling()
          )
        {
          const std::string name (name_element (child));

          if (name == "value")
            {
            }
          else if (name == "field")
            {
              cout << "field: "; token_field_type (child);
            }
          else
            {
              throw exception::unexpected_element ("token_type", name);
            }
        }
    }

    static void
    transition_type (const xml_node_type * node)
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
          const std::string name (name_element (child));

          if (name == "defun")
            {
              if (got_use)
                {
                  throw exception::error 
                    ("transition_type", "use and defun given at the same time");
                }

              cout << "defun: "; function_type (child);
            }
          else if (name == "connect-in")
            {
              cout << "connect_in: "; connect_type(child);
            }
          else if (name == "connect-out")
            {
              cout << "connect_out: "; connect_type(child);
            }
          else if (name == "connect-read")        
            {
              cout << "connect_read: "; connect_type(child);
            }
          else
            {
              throw exception::unexpected_element ("transition_type", name);
            }
        }
    }

    // ********************************************************************* //

    static void
    parse (std::istream & f)
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

          const std::string name (name_element (node));
      
          if (name == "defun")
            {
              cout << "defun: "; function_type (node);
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
  xml::parse::parse (std::cin);

  //  std::ifstream f ("/u/r/rahn/SDPA/trunk/xml/example/kdm/simple_kdm.xml");
  //  xml::parse::parse (f);

  return EXIT_SUCCESS;
}
