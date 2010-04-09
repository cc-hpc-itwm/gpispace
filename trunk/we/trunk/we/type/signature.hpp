// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_SIGNATURE_HPP
#define _WE_TYPE_SIGNATURE_HPP

#include <we/type/control.hpp>

#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>

#include <string>

namespace signature
{
  typedef std::string type_name_t;
  typedef std::string field_name_t;

  typedef boost::unordered_map<field_name_t,type_name_t> structured_t;

  typedef boost::variant<control, type_name_t, structured_t> type;

  class visitor_arity : public boost::static_visitor<std::size_t>
  {
  public:
    std::size_t operator () (const control &) { return 0; }
    std::size_t operator () (const type_name_t &) { return 1; }
    std::size_t operator () (const structured_t & m) { return m.size(); }
  };
}

#endif
