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

    const value::type& context::value (const std::string& key) const
    {
      return value::container::value (container, key);
    }

    const value::type& context::value (const key_vec_t& key_vec) const
    {
      return value::container::value (container, key_vec);
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
