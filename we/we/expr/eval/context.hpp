// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_CONTEXT_HPP
#define _EXPR_EVAL_CONTEXT_HPP

#include <we/expr/parse/node.hpp>

#include <we/type/value/container/container.hpp>

#include <fhg/util/rmap.hpp>

#include <iosfwd>

namespace expr
{
  namespace eval
  {
    struct context
    {
    private:
      typedef value::container::type container_t;
      container_t container;

      typedef fhg::util::rmap::traits< std::string
                                     , value::type const*
                                     > ref_traits;

      ref_traits::rmap_type _ref_container;

    public:
      typedef value::container::key_vec_t key_vec_t;
      typedef container_t::const_iterator const_iterator;

      void bind (const key_vec_t&, const value::type&);
      void bind (const std::string&, const value::type&);

      void bind_ref (const key_vec_t&, const value::type&);
      void bind_ref (const std::string&, const value::type&);

      const value::type& value (const std::string&) const;
      const value::type& value (const key_vec_t&) const;

      value::type clear();

      const_iterator begin() const;
      const_iterator end() const;
      std::size_t size() const;

      friend std::ostream& operator<< (std::ostream&, const context&);
    };

    std::ostream& operator<< (std::ostream&, const context&);

    parse::node::type refnode_value ( const context&
                                    , const context::key_vec_t&
                                    );
    parse::node::type refnode_name (const context::key_vec_t&);
  }
}

#endif
