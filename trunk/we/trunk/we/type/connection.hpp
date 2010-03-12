// mirko.rahn@itwm.fraunhofer.de

#ifndef _TYPE_CONNECTION_HPP
#define _TYPE_CONNECTION_HPP

#include <boost/serialization/nvp.hpp>

namespace petri_net
{
  template<typename TYPE, typename TID, typename PID>
  struct connection
  {
  public:
    TYPE type;
    TID tid;
    PID pid;

    connection () : type(), tid(), pid() {}

    connection (const TYPE & _type, const TID & _tid, const PID & _pid)
      : type (_type)
      , tid (_tid)
      , pid (_pid)
    {}

    template<typename Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(type);
      ar & BOOST_SERIALIZATION_NVP(tid);
      ar & BOOST_SERIALIZATION_NVP(pid);
    }
  };

  namespace detail
  {
	namespace tag {
	  struct p2t_tag {};
	  struct t2p_tag {};
	}

	template <typename TID, typename PID, typename Dir>
	struct tagged_connection
	{
	  typedef Dir direction;
	};
  }
}

#endif
