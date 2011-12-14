// bernd.loerwald@itwm.fraunhofer.de

#ifndef _EXPR_PARSE_SIMPLIFY_UTIL_HPP
#define _EXPR_PARSE_SIMPLIFY_UTIL_HPP 1

#include <we/expr/parse/parser.hpp>
#include <we/expr/parse/util/get_names.hpp>

#include <algorithm>

namespace expr
{
  namespace parse
  {
    namespace simplify
    {
      typedef node::key_vec_t key_type;
      typedef ::expr::parse::util::name_set_t key_set_type;

      namespace util
      {
        template<typename Vec>
        inline bool
        begins_with (const Vec & lhs, const Vec & rhs)
        {
          return lhs.size() >= rhs.size()
              && std::equal (rhs.begin(), rhs.end(), lhs.begin());
        }

        template<typename Map>
        inline void
        remove_parents_and_children_left ( const key_type & var
                                         , Map & map
                                         )
        {
          key_type current_parent (var);
          while (!current_parent.empty())
          {
            map.erase (current_parent);
            current_parent.pop_back();
          }

          for( typename Map::iterator it (map.begin()), end (map.end())
             ; it != end
             ;
             )
          {
            if (begins_with (it->first, var))
            {
              it = map.erase(it);
            }
            else
            {
              ++it;
            }
          }
        }

       template<typename Map>
        inline void
        remove_parents_and_children_both ( const key_type & var
                                         , Map & map
                                         )
        {
          key_type current_parent (var);
          while (!current_parent.empty())
          {
            for( typename Map::iterator it (map.begin()), end (map.end())
               ; it != end
               ;
               )
            {
              if (it->first == current_parent || it->second == current_parent)
              {
                it = map.erase(it);
              }
              else
              {
                ++it;
              }
            }
            current_parent.pop_back();
          }

          for( typename Map::iterator it (map.begin()), end (map.end())
             ; it != end
             ;
             )
          {
            if (begins_with (it->first, var) || begins_with (it->second, var))
            {
              it = map.erase(it);
            }
            else
            {
              ++it;
            }
          }
        }
      }
    }
  }
}

#endif
