// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_CONTEXT_HPP
#define _EXPR_EVAL_CONTEXT_HPP

#include <we/expr/parse/node.hpp>

#include <we/type/value.hpp>
#include <we/type/value/put.hpp>

#include <we/type/value/field.hpp>
#include <we/type/value/mk_structured.hpp>
#include <we/type/value/get.hpp>

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
      class missing_binding : public std::runtime_error
      {
      public:
        explicit missing_binding (const std::string & key)
          : std::runtime_error
            ("missing binding for: ${" + key + "}")
        {};
      };
    }
  }

  namespace eval
  {
    struct context
    {
    public:
      typedef expr::token::key_vec_t key_vec_t;

    private:
      typedef boost::unordered_map<std::string,value::type> container_t;
      container_t container;

      const value::type & find ( key_vec_t::const_iterator pos
                               , const key_vec_t::const_iterator end
                               , const value::type & store
                               ) const
      {
        if (pos == end)
          {
            return store;
          }
        else
          {
            value::visitor::get_field get (fhg::util::show (*pos));

            return find ( pos + 1
                        , end
                        , boost::apply_visitor (get, store)
                        );
          }
      }

    public:
      typedef container_t::const_iterator const_iterator;
      typedef container_t::iterator iterator;

      template<typename Path>
      value::type bind ( const std::string & key
                       , const Path & path
                       , const value::type & value
                       )
      {
        container[key]
          =  boost::apply_visitor ( value::visitor::mk_structured()
                                  , container[key]
                                  );

        return value::put (path, container[key], value);
      }

      value::type bind (const key_vec_t & key_vec, const value::type & value)
      {
        if (key_vec.size() == 0)
          {
            throw std::runtime_error ("context.bind []");
          }

        container[key_vec[0]] =
          boost::apply_visitor ( value::visitor::mk_structured()
                               , container[key_vec[0]]
                               );

        return value::put ( key_vec.begin() + 1
                          , key_vec.end()
                          , container[key_vec[0]]
                          , value
                          );
      }

      value::type bind (const std::string & key, const value::type & value)
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
                throw exception::eval::missing_binding (key_vec[0]);
              else
                return find ( key_vec.begin() + 1
                            , key_vec.end()
                            , pos->second
                            );
            }
          }
      }

      const value::type & value (const std::string & key) const
      {
        const const_iterator pos (container.find (key));

        if (pos == container.end())
          throw exception::eval::missing_binding (key);
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

      friend std::ostream & operator << (std::ostream &, const context &);
    };

    inline std::ostream & operator << (std::ostream & s, const context & cntx)
    {
      for ( context::const_iterator it (cntx.begin())
          ; it != cntx.end()
          ; ++it
          )
        s << it->first << " := " << it->second << std::endl;

      return s;
    }

    inline parse::node::type
    refnode_value ( const context & context
                  , const context::key_vec_t & key_vec
                  )
    {
      return parse::node::type (context.value(key_vec));
    }

    inline parse::node::type
    refnode_name (const context::key_vec_t & key_vec)
    {
      return parse::node::type (key_vec);
    }
  }
}

#endif
