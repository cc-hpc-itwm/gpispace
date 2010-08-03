// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_CONTEXT_HPP
#define _EXPR_EVAL_CONTEXT_HPP

#include <we/expr/parse/node.hpp>

#include <we/type/value.hpp>

#include <fhg/util/show.hpp>

#include <iostream>
#include <stdexcept>
#include <vector>

#include <boost/unordered_map.hpp>

namespace expr
{
  namespace exception
  {
    namespace eval
    {
      template<typename Key>
      class missing_binding : public std::runtime_error
      {
      public:
        explicit missing_binding (const Key & key)
          : std::runtime_error
            ("missing binding for: ${" + fhg::util::show(key) + "}")
        {};
      };
    }
  }

  namespace eval
  {
    template<typename Key>
    struct context
    {
    public:
      typedef std::vector<Key> key_vec_t;
    private:
      typedef boost::unordered_map<Key,value::type> container_t;
      container_t container;

      void modify ( typename key_vec_t::const_iterator pos
                  , const typename key_vec_t::const_iterator end
                  , value::type & store
                  , const value::type & value
                  )
      {
        ++pos;

        if (pos == end)
          store = value;
        else
          {
            store = boost::apply_visitor ( value::visitor::mk_structured()
                                         , store
                                         );

            modify ( pos
                   , end
                   , boost::apply_visitor
                     ( value::visitor::field (fhg::util::show (*pos))
                     , store
                     )
                   , value
                   );
          }
      }

      const value::type & find ( typename key_vec_t::const_iterator pos
                               , const typename key_vec_t::const_iterator end
                               , const value::type & store
                               ) const
      {
        ++pos;

        if (pos == end)
          return store;
        else
          return find ( pos
                      , end
                      , boost::apply_visitor
                        ( value::visitor::get_field (fhg::util::show (*pos))
                        , store
                        )
                      );
      }

    public:
      typedef typename container_t::const_iterator const_iterator;
      typedef typename container_t::iterator iterator;

      value::type bind (const key_vec_t & key_vec, const value::type & value)
      {
        switch (key_vec.size())
          {
          case 0:
            throw std::runtime_error ("context.bind []");
          case 1:
            container[key_vec[0]] = value;
          default:
            container[key_vec[0]] =
              boost::apply_visitor ( value::visitor::mk_structured()
                                   , container[key_vec[0]]
                                   );

            modify ( key_vec.begin()
                   , key_vec.end()
                   , container[key_vec[0]]
                   , value
                   );
          }
        return value;
      }

      value::type bind (const Key & key, const value::type & value)
      {
        container[key] = value; return value;
      }

      const value::type & value (const key_vec_t & key_vec) const
      {
        switch (key_vec.size())
          {
          case 0:
            throw std::runtime_error ("context.value []");
          default:
            {
              const const_iterator pos (container.find (key_vec[0]));

              if (pos == container.end())
                throw exception::eval::missing_binding<Key> (key_vec[0]);
              else
                return find ( key_vec.begin()
                            , key_vec.end()
                            , pos->second
                            );
            }
          }
      }

      const value::type & value (const Key & key) const
      {
        const const_iterator pos (container.find (key));

        if (pos == container.end())
          throw exception::eval::missing_binding<Key> (key);
        else
          return pos->second;
      }

      value::type clear ()
      {
        container.clear();
        return control();
      }

      const_iterator begin (void) const { return container.begin(); }
      const_iterator end (void) const { return container.end(); }
      std::size_t size (void) const { return container.size(); }

      template<typename K>
      friend std::ostream & operator << (std::ostream &, const context<K> &);
    };

    template<typename K>
    std::ostream & operator << (std::ostream & s, const context<K> & cntx)
    {
      for ( typename context<K>::const_iterator it (cntx.begin())
          ; it != cntx.end()
          ; ++it
          )
        s << it->first << " := " << it->second << std::endl;

      return s;
    }

    template<typename Key>
    parse::node::type<Key>
    refnode_value ( const context<Key> & context
                  , const std::vector<Key> & key_vec
                  )
    {
      return parse::node::type<Key>(context.value(key_vec));
    }

    template<typename Key>
    parse::node::type<Key> refnode_name (const std::vector<Key> & key_vec)
    {
      return parse::node::type<Key>(key_vec);
    }
  }
}

#endif
