// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/eval/context.hpp>

#include <we/type/value/container.hpp>
#include <we/type/value/find.hpp>
#include <we/type/value/put.hpp>
#include <we/type/value/mk_structured.hpp>

#include <iostream>

namespace expr
{
  namespace eval
  {
    void context::bind ( const std::list<std::string>& key_vec
                       , const value::type& value
                       )
    {
      if (key_vec.empty())
      {
        throw std::runtime_error ("context::bind []");
      }

      std::list<std::string>::const_iterator pos (key_vec.begin());
      const std::string& key (*pos); ++pos;

      value::put ( pos
                 , key_vec.end()
                 , value::mk_structured_or_keep (_container[key])
                 , value
                 );
    }

    void context::bind (const std::string& key, const value::type& value)
    {
      value::container::bind (_container, key, value);
    }

    void context::bind_ref (const std::string& key, const value::type& value)
    {
      _ref_container.insert (std::make_pair (key, &value));
    }

    const value::type& context::value (const std::string& key) const
    {
      try
      {
        return value::container::value (_container, key);
      }
      catch (const std::runtime_error&)
      {
        ref_container_type::const_iterator pos (_ref_container.find (key));

        if (pos != _ref_container.end())
        {
          return *pos->second;
        }

        throw;
      }
    }

    const value::type& context::value (const std::list<std::string>& key_vec) const
    {
      try
      {
        if (key_vec.empty())
        {
          throw std::runtime_error ("value::container::value []");
        }

        std::list<std::string>::const_iterator key_pos (key_vec.begin());
        const std::string& key (*key_pos); ++key_pos;

        const value::container::type::const_iterator pos (_container.find (key));

        if (pos == _container.end())
        {
          throw value::container::exception::missing_binding (key);
        }
        else
        {
          return value::find (key_pos, key_vec.end(), pos->second);
        }
      }
      catch (const std::runtime_error&)
      {
        if (key_vec.size() > 0)
        {
          std::list<std::string>::const_iterator key_pos (key_vec.begin());
          const std::string& key (*key_pos); ++key_pos;

          ref_container_type::const_iterator pos (_ref_container.find (key));

          if (pos != _ref_container.end())
          {
            if (key_pos == key_vec.end())
            {
              return *pos->second;
            }
            else
            {
              return value::find (key_pos, key_vec.end(), *pos->second);
            }
          }
        }

        throw;
      }
    }

    value::type context::clear()
    {
      _container.clear();
      _ref_container.clear();
      return we::type::literal::control();
    }

    context::const_iterator context::begin() const
    {
      return _container.begin();
    }

    context::const_iterator context::end() const
    {
      return _container.end();
    }

    std::size_t context::size() const
    {
      return _container.size();
    }

    std::ostream& operator<< (std::ostream& s, const context& cntx)
    {
      return s << cntx._container;
    }

    parse::node::type refnode_value ( const context& context
                                    , const std::list<std::string>& key_vec
                                    )
    {
      return parse::node::type (context.value (key_vec));
    }

    parse::node::type refnode_name (const std::list<std::string>& key_vec)
    {
      return parse::node::type (key_vec);
    }
  }
}
