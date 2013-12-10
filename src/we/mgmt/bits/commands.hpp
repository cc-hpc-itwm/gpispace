// alexander.petry@itwm.fraunhofer.de

#ifndef WE_MGMT_BITS_COMMANDS_HPP
#define WE_MGMT_BITS_COMMANDS_HPP 1

#include <boost/function.hpp>

namespace we { namespace mgmt { namespace detail { namespace commands {
  template <typename D>
  struct command_t
  {
	typedef command_t<D> this_type;
	typedef boost::function<void (this_type const &)> handler_type;

        command_t ()
        {}

	template <typename H>
	command_t(std::string const & n, D d, H h)
          : name(n)
	  , dat(d)
	  , handler(h)
	{}

        std::string name;
	D dat;

	void handle()
	{
	  handler(*this);
	}

  private:
	handler_type handler;
  };

  template <typename D, typename H>
  inline command_t<D> make_cmd(std::string const &n, D d, H h)
  {
        return command_t<D>(n, d, h);
  }
}}}}

#endif
