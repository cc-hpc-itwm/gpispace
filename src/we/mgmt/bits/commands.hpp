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
  enum CMD_ID
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

        command_t ()
        {}

	template <typename H>
	command_t(std::string const & n, C c, D d, H h)
          : name(n)
          , cmd(c)
	  , dat(d)
	  , handler(h)
	{}

        std::string name;
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
  inline command_t<CMD_ID, D> make_cmd(std::string const &n, D d, H h)
  {
        return command_t<CMD_ID,D>(n, GENERIC, d, h);
  }

  template <typename Stream, typename C, typename D>
  inline Stream & operator << (Stream & s, const command_t<C, D> & c)
  {
	return s << "cmd[" << c.cmd << ", " << c.dat << "]";
  }
}}}}

#endif
