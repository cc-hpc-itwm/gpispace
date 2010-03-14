/*
 * =====================================================================================
 *
 *       Filename:  signal.hpp
 *
 *    Description:  signal wrapper to implement observer pattern
 *
 *        Version:  1.0
 *        Created:  03/13/2010 03:03:32 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */


#ifndef WE_MGMT_BITS_SIGNAL_HPP
#define WE_MGMT_BITS_SIGNAL_HPP 1

#include <vector>
#include <boost/function.hpp>

namespace we { namespace mgmt { namespace detail {
  template <typename F = void (void) >
  class signal
  {
  public:
	template <typename C>
	void connect(C f)
	{
	  targets_.push_back(f);
	}

	void operator() ()
	{
	  typedef typename std::vector<boost::function<F> > funcs_t;
	  for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
	  {
		(*t)();
	  }
	}
	
	template <typename Arg1>
	void operator() (Arg1 a1)
	{
	  typedef typename std::vector<boost::function<F> > funcs_t;
	  for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
	  {
		(*t)(a1);
	  }
	}

	template <typename Arg1, typename Arg2>
	void operator() (Arg1 a1, Arg2 a2)
	{
	  typedef typename std::vector<boost::function<F> > funcs_t;
	  for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
	  {
		(*t)(a1, a2);
	  }
	}

	template <typename Arg1, typename Arg2, typename Arg3>
	void operator() (Arg1 a1, Arg2 a2, Arg3 a3)
	{
	  typedef typename std::vector<boost::function<F> > funcs_t;
	  for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
	  {
		(*t)(a1, a2, a3);
	  }
	}

  private:
	std::vector<boost::function<F> > targets_;
  };
}}}

#endif
