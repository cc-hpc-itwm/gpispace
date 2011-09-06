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

      class tree_node_type
        : public boost::enable_shared_from_this<tree_node_type>
      {
        private:
          typedef boost::shared_ptr<tree_node_type> ptr;

          typedef std::size_t version_type;
          typedef std::pair<version_type,line_type> item_type;
          typedef std::vector<item_type> values_type;
          typedef boost::unordered_map< key_part_type
                                      , tree_node_type::ptr > childs_type;


          values_type _values;
          childs_type _childs;

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

          inline tree_node_type::ptr
          insert_child (const key_part_type & name)
          {
            tree_node_type::ptr temp (tree_node_type::ptr (new tree_node_type()));
            _childs[name] = temp;
            for ( values_type::const_iterator it (_values.begin())
                , end (_values.end())
                ; it != end
                ; ++it
                )
            {
              temp->initialize (it->second);
            }
            return temp;
          }

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
              insert_child (*pos);
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

          void
          fill_asterics_with_latest ( const key_type::iterator & number
                                    , const key_type::iterator & end
                                    , const line_type & last_parent_line
                                    )
          {
            values_type::iterator value (_values.begin());
            //! \note Yes, I assume that there are no bad entries.
            while (value->second != last_parent_line)
            {
              ++value;
            }

            if (*number == "*")
            {
              // breaks when either last entry or next entry is (0, *).
              while ((value + 1) != _values.end() && (value + 1)->first != 0)
              {
                ++value;
              }
              *number = boost::lexical_cast<key_part_type> (value->first);
            }
            else
            {
              version_type wanted_version
                  (boost::lexical_cast<version_type>(*number));

              while (value->first != wanted_version)
              {
                ++value;
              }
            }

            line_type & last_line (value->second);

            if (number + 1 == end)
            {
              return;
            }

            const key_part_type & name (*(number + 1));

            if (!has_child (name))
            {
              key_type ssa_free
                  (util::get_normal_name (key_type (number + 1, end)));
              insert_child (ssa_free.begin(), ssa_free.end());
            }

            _childs[name]->fill_asterics_with_latest
                (number + 2, end, last_line);
          }

          inline line_type &
          iterate_to_given_version ( values_type::iterator & it
                                   , const line_type & last_parent_line
                                   , const version_type & wanted_version
                                   ) const

          {
            //! \note Yes, I assume that there are no bad entries.
            while (it->second != last_parent_line)
            {
              ++it;
            }

            while (it->first != wanted_version)
            {
              ++it;
            }

            return it->second;
          }

          line_type
          get_line_of_next_write ( const key_type::const_iterator & number
                                 , const key_type::const_iterator & end
                                 , const line_type & last_parent_line
                                 )
          {
            version_type wanted_version
                (boost::lexical_cast<version_type>(*number));

            values_type::iterator value (_values.begin());

            line_type & last_line
                (iterate_to_given_version ( value
                                          , last_parent_line
                                          , wanted_version
                                          ));

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

            return _childs[name]->get_line_of_next_write ( number + 2
                                                         , end
                                                         , last_line
                                                         );
          }

          tree_node_type::ptr
          get_entry_and_line ( const key_type::const_iterator & number
                             , const key_type::const_iterator & end
                             , line_type & last_parent_line
                             )
          {
            version_type wanted_version
                (boost::lexical_cast<version_type>(*number));

            values_type::iterator value (_values.begin());

            last_parent_line = iterate_to_given_version
                (value, last_parent_line, wanted_version);

            if (number + 1 == end)
            {
              return shared_from_this();
            }

            const key_part_type & name (*(number + 1));

            if (!has_child (name))
            {
              key_type ssa_free
                  (util::get_normal_name (key_type (number + 1, end)));
              insert_child (ssa_free.begin(), ssa_free.end());
            }

            return _childs[name]->get_entry_and_line ( number + 2
                                                     , end
                                                     , last_parent_line
                                                     );
          }

          void
          copy_children ( std::size_t parent_versions
                        , line_type & starting_at_line
                        , tree_node_type::ptr other
                        , line_type & starting_at_line_other
                        )
          {
            values_type::iterator start_of_range (_values.begin());
            while (start_of_range->second != starting_at_line)
            {
              ++start_of_range;
            }
            ++start_of_range;

            std::size_t my_versions (1);
            values_type::iterator end_of_range (start_of_range);
            while (end_of_range != _values.end() && parent_versions)
            {
              if ((end_of_range + 1)->first == 0)
              {
                --parent_versions;
              }
              if (parent_versions)
              {
                ++end_of_range;
                ++my_versions;
              }
            }

            other->_values.insert ( other->_values.end ()
                                  , start_of_range
                                  , end_of_range
                                  );

            for ( childs_type::const_iterator child (_childs.begin())
                , end (_childs.end())
                ; child != end
                ; ++child
                )
            {
              tree_node_type::ptr new_child (other->insert_child (child->first));
              child->second->copy_children ( my_versions
                                           , starting_at_line
                                           , new_child
                                           , starting_at_line_other
                                           );
            }
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
            get_current_name (key.begin (), key.end (), name);
            return name;
          }

          inline const line_type &
          get_last_line_of (const key_type & key)
          {
            return get_entry (key.begin (), key.end ())->_values.back().second;
          }

          inline line_type
          get_line_of_next_write (const key_type & key)
          {
            return get_line_of_next_write ( key.begin ()
                                          , key.end ()
                                          , line_type ()
                                          );
          }

          inline void
          fill_asterics_with_latest (key_type & key)
          {
            fill_asterics_with_latest (key.begin(), key.end(), line_type());
          }

          inline void
          latest_version_is_a_copy ( const key_type & target_key
                                   , const key_type & source_key
                                   )
          {
            line_type target_line;
            line_type source_line;
            tree_node_type::ptr target
                (get_entry_and_line ( target_key.begin ()
                                    , target_key.end ()
                                    , target_line
                                    )
                );
            tree_node_type::ptr source
                (get_entry_and_line ( source_key.begin ()
                                    , source_key.end ()
                                    , source_line
                                    )
                );

            source->copy_children ( 0 // copying the current version only.
                                  , source_line
                                  , target
                                  , target_line
                                  );
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
