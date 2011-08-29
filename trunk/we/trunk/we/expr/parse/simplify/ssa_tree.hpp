// bernd.loerwald@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_SIMPLIFY_SSA_TREE_HPP
#define _EXPR_PARSE_SIMPLIFY_SSA_TREE_HPP 1

#include <boost/unordered_set.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <we/expr/parse/node.hpp>
#include <we/expr/parse/simplify/expression_list.hpp>
#include <we/type/value.hpp>

#include <fhg/util/xml.hpp>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      typedef util::name_set_t::value_type key_type;
      typedef key_type::value_type key_part_type;
      typedef expression_list::nodes_type::iterator line_type;

      class variable_not_found : public std::exception
      {
        private:
          const std::string _what;
        public:
          variable_not_found() : _what ("variable not found") {}
          variable_not_found(const std::string & reason)
            : _what ("variable not found: " + reason) {}
          virtual ~variable_not_found() throw() {}
          virtual const char* what() const throw() { return _what.c_str(); }
      };

      class tree_node_type
        : public boost::enable_shared_from_this<tree_node_type>
      {
        private:
          typedef boost::shared_ptr<tree_node_type> ptr;
          typedef boost::shared_ptr<tree_node_type> const_ptr;

          typedef std::size_t version_type;
          typedef std::pair<version_type,line_type> item_type;
          typedef std::vector<item_type> values_type;
          typedef boost::unordered_map< key_part_type
                                      , tree_node_type::ptr > childs_type;

          inline void
          initialize_childs (const line_type & line)
          {
            for ( childs_type::iterator child (_childs.begin())
                , end (_childs.end())
                ; child != end
                ; ++child
                )
            {
              child->second->initialize (line);
            }
          }

          inline bool
          has_child (const key_part_type & part) const
          {
            return _childs.count (part) != 0;
          }

          values_type _values;
          childs_type _childs;

          tree_node_type::ptr
          insert_child ( const key_type::const_iterator & pos
                       , const key_type::const_iterator & end
                       )
          {
            if (pos == end)
            {
              return shared_from_this();
            }

            if (!has_child (*pos))
            {
              _childs[*pos] = tree_node_type::ptr (new tree_node_type());
              for ( values_type::const_iterator it (_values.begin())
                  , end (_values.end())
                  ; it != end
                  ; ++it
                  )
              {
                _childs[*pos]->initialize (it->second);
              }
            }

            return _childs[*pos]->insert_child (pos + 1, end);
          }

          void
          get_current_name ( const key_type::const_iterator & pos
                           , const key_type::const_iterator & end
                           , key_type & name
                           )
          {
            name.push_back
                (boost::lexical_cast<key_part_type> (_values.back().first));

            if (pos == end)
            {
              return;
            }

            name.push_back (*pos);

            if (!has_child (*pos))
            {
              insert_child (pos, end);
            }

            _childs[*pos]->get_current_name (pos + 1, end, name);
          }

          tree_node_type::ptr
          get_entry ( const key_type::const_iterator & pos
                    , const key_type::const_iterator & end
                    )
          {
            if (pos == end)
            {
              return shared_from_this();
            }

            if (has_child (*pos))
            {
              return _childs[*pos]->get_entry (pos + 1, end);
            }

            return insert_child (pos, end);
          }

          const line_type &
          get_line_of ( const key_type::const_iterator & number
                      , const key_type::const_iterator & end
                      , const line_type & last_parent_line
                      )
          {
            version_type wanted_version
                (boost::lexical_cast<version_type>(*number));

            values_type::iterator value (_values.begin());
            //! \note Yes, I assume that there are no bad entries.
            while (value->second != last_parent_line)
            {
              ++value;
            }

            while (value->first != wanted_version)
            {
              ++value;
            }

            line_type & last_line (value->second);

            if (number + 1 == end)
            {
              values_type::iterator next_value (value + 1);
              if (next_value != _values.end())
              {
                return next_value->second;
              }
              return line_type ();
            }

            const key_part_type & name (*(number + 1));

            if (!has_child (name))
            {
              key_type ssa_free
                  (util::get_normal_name (key_type (number + 1, end)));
              insert_child (ssa_free.begin(), ssa_free.end());
            }

            return _childs[name]->get_line_of (number + 2, end, last_line);
          }

          void
          dump (fhg::util::xml::xmlstream & stream) const
          {
            for( values_type::const_iterator it (_values.begin())
               , end (_values.end())
               ; it != end
               ; ++it )
            {
              stream.open ("ssa");
              stream.attr ("number", it->first);
              //stream.attr ("line", ...);
              stream.close();
            }
            for ( childs_type::const_iterator child (_childs.begin())
                , end (_childs.end())
                ; child != end
                ; ++child
                )
            {
              stream.open ("child");
              stream.attr ("name", child->first);
              child->second->dump (stream);
              stream.close();
            }
          }

        public:
          explicit tree_node_type () : _values (0), _childs (0)
          {}

          inline void
          add_reference (const line_type & line)
          {
            version_type version (_values.back().first);

            _values.push_back (item_type (++version, line));

            initialize_childs (line);
          }

          inline void
          add_reference (const key_type & key, const line_type & line)
          {
            get_entry (key.begin (), key.end ())->add_reference (line);
          }

          inline void
          initialize (const line_type & line = line_type ())
          {
            _values.push_back (item_type (version_type(), line));

            initialize_childs (line);
          }

          inline key_type
          get_current_name (const key_type & key)
          {
            key_type name;
            get_current_name (key.begin(), key.end(), name);
            return name;
          }

          inline const line_type &
          get_last_line_of (const key_type & key)
          {
            return get_entry (key.begin (), key.end ())->_values.back().second;
          }

          inline const line_type &
          get_line_of (const key_type & key)
          {
            return get_line_of (key.begin (), key.end (), line_type());
          }

          inline void
          dump (std::ostream & s = std::cout) const
          {
            fhg::util::xml::xmlstream stream (s);
            stream.open ("root");
            dump (stream);
            stream.close();
          }
      };
    }
  }
}

#endif
