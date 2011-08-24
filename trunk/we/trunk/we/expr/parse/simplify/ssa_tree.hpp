// bernd.loerwald@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_SIMPLIFY_SSA_TREE_HPP
#define _EXPR_PARSE_SIMPLIFY_SSA_TREE_HPP 1

#include <boost/unordered_set.hpp>
#include <boost/lexical_cast.hpp>

#include <we/expr/parse/node.hpp>
#include <we/expr/parse/simplify/expression_list.hpp>
#include <we/type/value.hpp>

#include <sstream>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      typedef std::string key_type;
      typedef std::size_t version_type;
      typedef expression_list::node_stack_it_t line_type;
      typedef std::pair<version_type,line_type> item_type;
      typedef std::vector<item_type> values_type;

      typedef util::name_set_t::value_type key_vec_t;

      struct tree_node_type;

      typedef boost::unordered_map<key_type, boost::shared_ptr<tree_node_type> > map_type;

      namespace helper
      {
        std::string indention (std::size_t indent);
      }

      struct tree_node_type
      {
        private:
          void reset_childs (const line_type & line)
          {
            for ( map_type::iterator child (childs.begin())
                ; child != childs.end()
                ; ++child
                )
            {
              child->second->reset (line);
            }
          }

          values_type values;
          map_type childs;

        public:
          tree_node_type (const line_type & begin)
          : values (0)
          , childs (0)
          {
            values.push_back (item_type (version_type (), begin));
          }

          void inc (const line_type & line)
          {
            version_type version (values.back().first);

            values.push_back (item_type (++version, line));

            reset_childs (line);
          }

          void reset (const line_type & line)
          {
            values.push_back (item_type (version_type (), line));

            reset_childs (line);
          }

          tree_node_type* find_child ( const key_vec_t::const_iterator & pos
                                     , const key_vec_t::const_iterator & end
                                     ) const
          {
            if (pos == end)
            {
              return const_cast<tree_node_type*>(this);
            }
            const map_type::const_iterator & child (childs.find (*pos));

            if (child != childs.end())
            {
              return child->second->find_child (pos + 1, end);
            }
            else
            {
              throw std::runtime_error ("variable not found");
            }
          }

          tree_node_type* find_child (const key_vec_t & keyvec) const
          {
            return find_child (keyvec.begin(), keyvec.end());
          }

          tree_node_type& add_child ( const key_vec_t::const_iterator & pos
                                    , const key_vec_t::const_iterator & end
                                    , const line_type & line
                                    )
          {
            if (pos == end)
            {
              return *this;
            }

            const map_type::const_iterator & child (childs.find (*pos));

            if (child == childs.end())
            {
              childs[*pos] = boost::shared_ptr<tree_node_type> ( new tree_node_type (line));
            }

            return childs[*pos]->add_child (pos + 1, end, line);
          }

          tree_node_type& add_child ( const key_vec_t & keyvec
                                    , const line_type & line
                                    )
          {
            return add_child (keyvec.begin(), keyvec.end(), line);
          }

          void get_ssa_name( const key_vec_t::const_iterator & pos
                           , const key_vec_t::const_iterator & end
                           , key_vec_t & name
                           ) const
          {
            name.push_back (boost::lexical_cast<key_vec_t::value_type> (values.back().first));

            if (pos == end)
            {
              return;
            }

            const map_type::const_iterator & child (childs.find (*pos));

            if (child != childs.end())
            {
              name.push_back (*pos);
              child->second->get_ssa_name (pos + 1, end, name);
            }
            else
            {
              throw std::runtime_error ("variable not found");
            }
          }

          key_vec_t get_ssa_name (const key_vec_t & keyvec) const
          {
            key_vec_t name;
            get_ssa_name (keyvec.begin(), keyvec.end(), name);
            return name;
          }

          void dump (std::size_t indention) const
          {
            const std::string indentstr (helper::indention (indention));

            for( values_type::const_iterator it (values.begin()), end (values.end())
               ; it != end
               ; ++it )
            {
              //! \todo Make this possible again, by passing in the iterator to begin() and use distance(begin, second).
              //std::cout << "[" << back.first << ", " << back.second << "]";
              std::cout << "[" << it->first << "]";
            }
            if (!childs.empty())
            {
              std::cout << " {\n";
              for ( map_type::const_iterator child (childs.begin()), end (childs.end())
                  ; child != end
                  ; /* due to fuckup in the const_iterator, we iterate when comparing below. */
                  )
              {
                std::cout << indentstr << "  " << child->first;
                child->second->dump (indention + 1);
                if( ++child != end )
                  std::cout << ",\n";
              }
              std::cout << "\n" << indentstr << "}";
            }
          }
      };

      namespace helper
      {
        std::string indention (std::size_t indent)
        {
          std::stringstream ss;
          while( indent-- )
          {
            ss << "  ";
          }
          return ss.str();
        }

        void dump (const tree_node_type & root)
        {
          root.dump (0);
          std::cout << std::endl;
        }
      }

      inline void
      inc (tree_node_type & root, const key_vec_t & key, const line_type & line)
      {
        root.find_child (key)->inc (line);
      }

      inline void
      add (tree_node_type & root, const key_vec_t & key, const line_type & line)
      {
        root.add_child (key, line);
      }

      inline tree_node_type
      create_from_name_set ( const util::name_set_t& set
                           , const line_type & line)
      {
        tree_node_type root (line);

        for ( util::name_set_t::const_iterator name = set.begin()
            ; name != set.end()
            ; ++name)
        {
          add( root, *name, line );
        }

        return root;
      }
    }
  }
}

#endif
