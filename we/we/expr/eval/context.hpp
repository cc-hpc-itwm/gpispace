// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_CONTEXT_HPP
#define _EXPR_EVAL_CONTEXT_HPP

#include <we/type/value.hpp>

#include <boost/unordered_map.hpp>

#include <list>
#include <string>

#include <iosfwd>

namespace expr
{
  namespace eval
  {
    struct context
    {
    private:
      typedef boost::unordered_map< std::string
                                  , value::type
                                  > container_type;

      typedef boost::unordered_map< std::string
                                  , value::type const*
                                  > ref_container_type;

      container_type _container;
      ref_container_type _ref_container;

    public:
      typedef std::list<std::string> key_vec_t;
      typedef container_type::const_iterator const_iterator;

      void bind (const std::list<std::string>&, const value::type&);
      void bind (const std::string&, const value::type&);
      void bind ( const std::string&, const std::list<std::string>&
                , const value::type&
                );
      void bind ( const std::string&, const std::string&
                , const value::type&
                );

      void bind_ref (const std::string&, const value::type&);

      const value::type& value (const std::string&) const;
      const value::type& value (const std::list<std::string>&) const;

      value::type clear();

      const_iterator begin() const;
      const_iterator end() const;
      std::size_t size() const;

      friend std::ostream& operator<< (std::ostream&, const context&);
    };

    std::ostream& operator<< (std::ostream&, const context&);
  }
}

#endif
