// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/parse/parser.hpp>

#include <we/expr/parse/util/get_names.hpp>

#include <iostream>
#include <string>

int
main (int argc, char ** argv)
{
  const std::string input
    ("${x.a} * ${y.a} - ${z.coord.phi} + ${x.b} / sin (${t} + pi)");

  expr::parse::parser parser (input);

  expr::parse::util::name_set_t names
    (expr::parse::util::get_names (parser.front()));

  typedef expr::parse::util::name_set_t name_set_t;

  for ( name_set_t::const_iterator name (names.begin())
      ; name != names.end()
      ; ++name
      )
    {
      //      add (root, name, line);

      for ( name_set_t::value_type::const_iterator field (name->begin())
          , end = name->end()
          ; field != end
          ; ++field
          )
        {
          std::cout << *field;

          if (field + 1 != end)
            {
              std::cout << ".";
            }
        }

      std::cout << std::endl;
    }

  return EXIT_SUCCESS;
}

/*
#include key_vec_t

typedef std::string key_type;
typedef std::size_t version_type;
typedef std::list<expr_type>::iterator line_type;
typedef std::pair<version_type,line_type> item_type;
typedef std::vector<item_type> value_type;

struct tree_node_type;

typedef boost::unordered_map<key_type, tree_node_type *> map_type

struct tree_node_type
{
private:
  void reset_childs (const line_type & line)
  {
    for ( map_type::iterator child (childs->begin())
        ; child != childs->end()
        ; ++child
        )
      {
        child->reset (line);
      }
  }

  value_type value;
  map_type childs;

public:
  tree_node_type (const line_type & begin)
    : value (0)
    , childs (0)
  {
    value.push_back (item_type (version_type (), begin));
  }

  void inc (const line_type & line)
  {
    const version_type & version (value.back().first);

    value.push_back (item_type (++version, line));

    reset_childs (line);
  }

  void reset (const line_type & line)
  {
    value.push_back (item_type (version_type (), line));

    reset_childs (line);
  }

  tree_node_type * find_child ( const key_vec_t::const_iterator & pos
                              , const key_vec_t::const_iterator & end
                              ) const
  {
    if (pos == end)
      {
        return this;
      }

    const map_type::const_iterator & child (childs.find (*pos));

    if (child != childs.end())
      {
        return find_child (pos + 1, end);
      }
    else
      {
        throw std::runtime_error ("variable not found");
      }
  }

  tree_node_type * find_child (const key_vec_t & keyvec) const
  {
    find_child (keyvec.begin(), keyvec.end());
  }

  tree_node_type * add_child ( const key_vec_t::const_iterator & pos
                             , const key_vec_t::const_iterator & end
                             , const line_type & line
                             )
  {
    if (pos == end)
      {
        return this;
      }

    const map_type::const_iterator & child (childs.find (*pos));

    if (child == childs.end())
      {
        childs[*pos] = tree_node_type (line);
      }

    return childs[*pos].add_child (pos + 1, end, line);
  }

  tree_node_type * add_child ( const key_vec_t & keyvec
                             , const line_type & line
                             )
  {
    add_child (keyvec.begin(), keyvec.end(), line);
  }
};

void inc (tree_node_type & root, const key_vec_t & key, const line_type & line)
{
  root.find_child(key).inc(line);
}

void add (tree_node_type & root, const key_vec_t & key, const line_type & line)
{
  root.add_child (key, line);
}
*/
