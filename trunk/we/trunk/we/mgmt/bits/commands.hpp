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

namespace we { namespace mgmt { namespace detail { namespace commands {
  enum E_CMD_ID
  {
	NET_NEEDS_ATTENTION
  , NET_SUSPEND
  , NET_RESUME
  , NET_CANCEL
  };

  template <typename C, typename D>
  struct command_t
  {
	command_t(C c, D d)
	  : cmd(c)
	  , dat(d)
	{}

	C cmd;
	D dat;
  };

  template <typename C, typename D>
  inline command_t<C,D> make_cmd(C c, D d)
  {
	return command_t<C,D>(c,d);
  }

  template <typename Stream, typename C, typename D>
  inline Stream & operator << (Stream & s, const command_t<C, D> & c)
  {
	return s << "cmd[" << c.cmd << ", " << c.dat << "]";
  }
}}}}

#endif
