/*
 * =====================================================================================
 *
 *       Filename:  commands.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/05/2010 04:05:14 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_BITS_COMMANDS_HPP
#define WE_MGMT_BITS_COMMANDS_HPP 1

#include <boost/function.hpp>

namespace we { namespace mgmt { namespace detail { namespace commands {
  enum E_CMD_ID
  {
	GENERIC
  ,	NET_NEEDS_ATTENTION
  , NET_SUSPEND
  , NET_RESUME
  , NET_CANCEL
  };

  template <typename C, typename D>
  struct command_t
  {
	typedef command_t<C,D> this_type;
	typedef boost::function<void (this_type const &)> handler_type;

	command_t(C c, D d)
	  : cmd(c)
	  , dat(d)
	{}

	template <typename H>
	command_t(C c, D d, H h)
	  : cmd(c)
	  , dat(d)
	  , handler(h)
	{}

	C cmd;
	D dat;

	void handle()
	{
	  handler(*this);
	}

  private:
	handler_type handler;
  };

  template <typename D, typename H>
  inline command_t<E_CMD_ID, D> make_cmd(D d, H h)
  {
	return command_t<E_CMD_ID,D>(GENERIC, d, h);
  }

  template <typename C, typename D, typename H>
  inline command_t<C,D> make_cmd(C c, D d, H h)
  {
	return command_t<C,D>(c,d, h);
  }

  template <typename Stream, typename C, typename D>
  inline Stream & operator << (Stream & s, const command_t<C, D> & c)
  {
	return s << "cmd[" << c.cmd << ", " << c.dat << "]";
  }
}}}}

#endif
