// mirko.rahn@itwm.fraunhofer.de

#include <we/expr/eval/context.hpp>

#include <we/type/value/container/bind.hpp>
#include <we/type/value/container/value.hpp>
#include <we/type/value/container/show.hpp>

#include <iostream>

namespace expr
{
  namespace eval
  {
    void context::bind (const key_vec_t& key_vec, const value::type& value)
    {
      value::container::bind (container, key_vec, value);
    }

    void context::bind (const std::string& key, const value::type& value)
    {
      value::container::bind (container, key, value);
    }

    void context::bind_ref (const std::string& key, const value::type& value)
    {
      _ref_container.insert (std::make_pair (key, &value));
    }

    const value::type& context::value (const std::string& key) const
    {
      try
      {
        return value::container::value (container, key);
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

    const value::type& context::value (const key_vec_t& key_vec) const
    {
      try
      {
        return value::container::value (container, key_vec);
      }
      catch (const std::runtime_error&)
      {
        if (key_vec.size() > 0)
        {
          key_vec_t::const_iterator key_pos (key_vec.begin());
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
              return value::container::find ( key_pos
                                            , key_vec.end()
                                            , *pos->second
                                            );
            }
          }
        }

        throw;
      }
    }

    value::type context::clear()
    {
      container.clear();
      return we::type::literal::control();
    }

    context::const_iterator context::begin() const
    {
      return container.begin();
    }

    context::const_iterator context::end() const
    {
      return container.end();
    }

    std::size_t context::size() const
    {
      return container.size();
    }

    std::ostream& operator<< (std::ostream& s, const context& cntx)
    {
      value::container::show (s, cntx.container);

      return s;
    }

    parse::node::type refnode_value ( const context& context
                                    , const context::key_vec_t& key_vec
                                    )
    {
      return parse::node::type (context.value (key_vec));
    }

    parse::node::type refnode_name (const context::key_vec_t& key_vec)
    {
      return parse::node::type (key_vec);
    }
  }
}
