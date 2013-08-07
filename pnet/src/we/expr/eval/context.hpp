// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EVAL_CONTEXT_HPP
#define _EXPR_EVAL_CONTEXT_HPP

#include <we/type/value.hpp>
#include <we2/type/value.hpp>

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
      typedef container_type::const_iterator const_iterator;

      void bind (const std::string&, const pnet::type::value::value_type&);
      void bind_ref (const std::string&, const pnet::type::value::value_type&);

      void bind (const std::string&, const value::type&);
      void bind_ref (const std::string&, const value::type&);

      void bind_and_discard_ref ( const std::list<std::string>&
                                , const value::type&
                                );

      const value::type& value (const std::string&) const;
      const value::type& value (const std::list<std::string>&) const;

      const boost::unordered_map<std::string,value::type>& values() const;

      friend std::ostream& operator<< (std::ostream&, const context&);

    private:
      void bind (const std::list<std::string>&, const value::type& value);
    };

    std::ostream& operator<< (std::ostream&, const context&);
  }
}

#endif
