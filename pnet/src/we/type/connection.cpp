// mirko.rahn@itwm.fraunhofer.de

#include <we/type/connection.hpp>

#include <boost/functional/hash.hpp>

#include <stdexcept>

namespace petri_net
{
  namespace edge
  {
    bool is_pt_read (const type& e)
    {
      return e == PT_READ;
    }
    bool is_PT (const type& e)
    {
      return (e == PT || e == PT_READ);
    }

    std::string enum_to_string (const type& e)
    {
      switch (e)
	{
	case PT: return "in";
	case PT_READ: return "read";
	case TP: return "out";
	}

      throw std::runtime_error ("enum_to_string for invalid enum value");
    }
  }

  connection_t::connection_t ()
    : _type()
    , _tid()
    , _pid()
  {}

  connection_t::connection_t ( const edge::type& type
	 		     , const transition_id_type& tid
		 	     , const place_id_type& pid
			     )
    : _type (type)
    , _tid (tid)
    , _pid (pid)
  {}

  std::size_t hash_value (const connection_t& c)
  {
    std::size_t h (0);
    boost::hash_combine (h, c.type());
    boost::hash_combine (h, c.transition_id());
    boost::hash_combine (h, c.place_id());

    return h;
  }

  bool operator== (const connection_t& a, const connection_t& b)
  {
    return a.type() == b.type()
      && a.transition_id() == b.transition_id()
      && a.place_id() == b.place_id();
  }
  bool operator!= (const connection_t& a, const connection_t& b)
  {
    return not (a == b);
  }
}
