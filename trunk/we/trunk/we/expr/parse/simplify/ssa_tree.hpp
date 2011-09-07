// bernd.loerwald@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_SIMPLIFY_SSA_TREE_HPP
#define _EXPR_PARSE_SIMPLIFY_SSA_TREE_HPP 1

/*#include <boost/unordered_set.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <we/expr/parse/node.hpp>
#include <we/type/value.hpp>

#include <fhg/util/xml.hpp>*/

/// new tree

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/unordered_map.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/lexical_cast.hpp>

#include <string>

#include <we/expr/parse/util/get_names.hpp>

#include <we/expr/parse/simplify/expression_list.hpp>

//! \note Use c++0x for having the static_assert below enabled.
#if __cplusplus > 199711L
#include <type_traits>
#endif

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      typedef util::name_set_t::value_type key_type;
      //! \note For readability only. As of now.
      typedef key_type ssa_key_type;
      typedef key_type::value_type key_part_type;
      typedef expression_list::nodes_type::iterator line_type;

      class ssa_tree_type : public boost::enable_shared_from_this<ssa_tree_type>
      {
        public:
          typedef boost::shared_ptr<ssa_tree_type> ptr;

        private:
          typedef boost::unordered_map< key_part_type
                                      , ssa_tree_type::ptr
                                      > childs_type;
          typedef boost::shared_ptr<childs_type> childs_ptr;
          typedef std::vector<childs_ptr> versioned_childs_type;
          typedef versioned_childs_type::size_type version_type;
          typedef std::vector<line_type> versioned_lines_type;

#if __cplusplus > 199711L
          static_assert( std::is_same< versioned_childs_type::size_type
                                     , versioned_lines_type::size_type
                                     >::value
                       , "index type for line and child containers not equal"
                       );
#endif

          versioned_childs_type _versioned_childs;
          versioned_lines_type _versioned_lines;

          inline bool
          has_child (const version_type& version, const key_part_type & part) const
          {
            return _versioned_childs[version]->count (part) != 0;
          }

          inline ssa_tree_type::ptr
          insert_child (const version_type & version, const key_part_type & n)
          {
            ssa_tree_type::ptr new_child (new ssa_tree_type ());
            _versioned_childs[version]->insert( childs_type::value_type
                                                                (n, new_child));
            return new_child;
          }

          ssa_tree_type::ptr
          insert_child_at_last_version ( const key_type::const_iterator & pos
                                       , const key_type::const_iterator & end
                                       )
          {
            if (pos == end)
            {
              return shared_from_this();
            }

            const version_type last_version (_versioned_childs.size() - 1u);

            if (!has_child (last_version, *pos))
            {
              return insert_child (last_version, *pos);
            }

            return (*_versioned_childs[last_version])[*pos]->
                      insert_child_at_last_version(pos + 1, end);
          }

          void
          get_current_name ( const key_type::const_iterator & pos
                           , const key_type::const_iterator & end
                           , key_type & name
                           )
          {
            const version_type last_version (_versioned_childs.size() - 1u);
            name.push_back (boost::lexical_cast<key_part_type> (last_version));

            if (pos == end)
            {
              return;
            }

            name.push_back (*pos);

            if (!has_child (last_version, *pos))
            {
              insert_child_at_last_version (pos, end);
            }

            (*_versioned_childs[last_version])[*pos]->get_current_name ( pos + 1
                                                                    , end
                                                                    , name
                                                                    );
          }

          void
          fill_asterics_with_latest ( const ssa_key_type::iterator & number
                                    , const ssa_key_type::const_iterator & end
                                    )
          {
            const version_type last_version (_versioned_childs.size() - 1u);
            const version_type recurse_version
              (*number == "*" ? last_version
                              : boost::lexical_cast<version_type> (*number)
              );
            *number = boost::lexical_cast<key_part_type> (recurse_version);

            if (number + 1 == end)
            {
              return;
            }

            const key_part_type & name (*(number + 1));

            if (!has_child (recurse_version, name))
            {
              throw std::runtime_error
                  ("fill_asterics_with_latest: trying to get ssa-numbering for an non-existant variable");
            }

            (*_versioned_childs[recurse_version])[name]->fill_asterics_with_latest
                                                              (number + 2, end);
          }

          ssa_tree_type::ptr
          get_entry_ssa ( const ssa_key_type::const_iterator & number
                        , const ssa_key_type::const_iterator & end
                        )
          {
            const version_type recurse_version
                (boost::lexical_cast<version_type> (*number));

            if (number + 1 == end)
            {
              if (recurse_version > _versioned_childs.size() - 1u)
              {
                throw std::runtime_error
                    ("get_entry_ssa: trying to get ssa-numbering for an non-existant variable");
              }
              return shared_from_this();
            }

            const key_part_type & name (*(number + 1));

            if (!has_child (recurse_version, name))
            {
              throw std::runtime_error
                  ("get_entry_ssa: trying to get ssa-numbering for an non-existant variable");
            }

            return (*_versioned_childs[recurse_version])[name]->get_entry_ssa
                                                              (number + 2, end);
          }

          inline ssa_tree_type::ptr
          get_entry_ssa (const ssa_key_type & key)
          {
            return get_entry_ssa (key.begin(), key.end());
          }

          inline ssa_tree_type::ptr
          get_entry (const key_type & key)
          {
            return insert_child_at_last_version (key.begin(), key.end());
          }

          inline void
          add_reference (const line_type & line)
          {
            _versioned_lines.push_back (line);
            _versioned_childs.push_back (childs_ptr (new childs_type ()));
          }

          void
          dump (fhg::util::xml::xmlstream & stream) const
          {
            version_type version (0);
            for( versioned_childs_type::const_iterator it
                                                     (_versioned_childs.begin())
               , end (_versioned_childs.end())
               ; it != end
               ; ++it )
            {
              stream.open ("version");
              stream.attr ("number", version++);
              for ( childs_type::const_iterator child ((*it)->begin())
                  , end ((*it)->end())
                  ; child != end
                  ; ++child
                  )
              {
                stream.open ("child");
                stream.attr ("name", child->first);
                child->second->dump (stream);
                stream.close();
              }
              stream.close();
            }
          }

        public:
          explicit ssa_tree_type ()
          : _versioned_childs (0u)
          , _versioned_lines (0u)
          {
            add_reference (line_type ());
          }

          inline void
          fill_asterics_with_latest (ssa_key_type & key)
          {
            fill_asterics_with_latest (key.begin(), key.end());
          }

          inline key_type
          get_current_name (const key_type & key)
          {
            key_type name;
            get_current_name (key.begin (), key.end (), name);
            return name;
          }

          inline void
          add_reference (const key_type & key, const line_type & line)
          {
            insert_child_at_last_version (key.begin(), key.end())->
                add_reference (line);
          }

          inline line_type
          get_line_of_next_write (const ssa_key_type & key)
          {
            std::cout << "get_line_of_next_write(" << key << ")\n";
            ssa_tree_type::ptr child (get_entry_ssa (key.begin(), key.end()));

            const version_type after
                (boost::lexical_cast<version_type> (key.back()));
            const version_type last
                (child->_versioned_lines.size () - 1u);

            if (after == last)
            {
              return line_type ();
            }

            return child->_versioned_lines[after + 1];
          }

          inline void
          copy_children (const ssa_key_type & from_k, const ssa_key_type & to_k)
          {
            std::cout << "copy_children(" << from_k << " -> " << to_k << ")\n";
            ssa_tree_type::ptr from
                (get_entry_ssa (from_k.begin(), from_k.end()));
            ssa_tree_type::ptr to (get_entry_ssa (to_k.begin(), to_k.end()));

            const version_type from_v
                (boost::lexical_cast<version_type> (from_k.back()));
            const version_type to_v
                (boost::lexical_cast<version_type> (to_k.back()));

            to->_versioned_childs[to_v] = from->_versioned_childs[from_v];
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


/// new tree

/*

namespace expr
{
  namespace parse
  {
    namespace simplify
    {

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

*/

#endif
