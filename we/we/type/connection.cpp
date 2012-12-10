// mirko.rahn@itwm.fraunhofer.de

#include <we/type/connection.hpp>

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
    : type()
    , tid()
    , pid()
  {}

  connection_t::connection_t ( const edge::type& _type
	 		     , const transition_id_type& _tid
		 	     , const pid_t& _pid
			     )
    : type (_type)
    , tid (_tid)
    , pid (_pid)
  {}
}
