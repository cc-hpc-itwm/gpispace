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

      container_type _container;

      typedef boost::unordered_map< std::string
                                  , pnet::type::value::value_type
                                  > container2_type;

      container2_type _container2;

    public:
      typedef container_type::const_iterator const_iterator;

      void BIND_OLD (const std::string&, const value::type&);

      void bind (const std::string&, const pnet::type::value::value_type&);
      void bind_ref (const std::string&, const pnet::type::value::value_type&);

      void bind_and_discard_ref ( const std::list<std::string>&
                                , const pnet::type::value::value_type&
                                );

      value::type value (const std::string&) const;
      value::type value (const std::list<std::string>&) const;

      const pnet::type::value::value_type& value2 (const std::string&) const;
      const pnet::type::value::value_type& value2 (const std::list<std::string>&) const;

      friend std::ostream& operator<< (std::ostream&, const context&);

      const boost::unordered_map<std::string,value::type>& values() const;
    };

    std::ostream& operator<< (std::ostream&, const context&);
  }
}

#endif
