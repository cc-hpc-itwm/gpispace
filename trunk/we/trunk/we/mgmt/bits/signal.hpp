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
#include <ostream>
#include <iostream>
#include <boost/function.hpp>

namespace we
{
  namespace mgmt
  {
    namespace util
    {
      template <typename F = void (void) >
      class signal
      {
      public:
        explicit
        signal(const std::string & name = "noname")
          : name_(name)
        { }

	template <typename C>
	void connect(C f)
	{
	  targets_.push_back(f);
	}

        void clear ()
        {
          targets_.clear();
        }

        const std::string &name() const { return name_; }

	void operator() ()
	{
          if (targets_.empty())
          {
            std::cerr << "W: " << name() << " not connected to anybody!" << std::endl;
            return;
          }

	  typedef typename std::vector<boost::function<F> > funcs_t;
	  for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
	  {
            (*t)();
	  }
	}

	template <typename Arg1>
	void operator() (Arg1 a1)
	{
          if (targets_.empty())
          {
            std::cerr << "W: " << name() << " not connected to anybody!" << std::endl;
            return;
          }

	  typedef typename std::vector<boost::function<F> > funcs_t;
	  for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
	  {
            (*t)(a1);
	  }
	}

	template <typename Arg1, typename Arg2>
	void operator() (Arg1 a1, Arg2 a2)
	{
          if (targets_.empty())
          {
            std::cerr << "W: " << name() << " not connected to anybody!" << std::endl;
            return;
          }
	  typedef typename std::vector<boost::function<F> > funcs_t;
	  for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
	  {
            (*t)(a1, a2);
	  }
	}

	template <typename Arg1, typename Arg2, typename Arg3>
	void operator() (Arg1 a1, Arg2 a2, Arg3 a3)
	{
          if (targets_.empty())
          {
            std::cerr << "W: " << name() << " not connected to anybody!" << std::endl;
            return;
          }
	  typedef typename std::vector<boost::function<F> > funcs_t;
	  for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
	  {
            (*t)(a1, a2, a3);
	  }
	}

      private:
        std::string name_;
	std::vector<boost::function<F> > targets_;
      };

      template <typename F>
      inline std::ostream & operator << (std::ostream & os, const signal<F> & sig)
      {
        os << "SIG_" << sig.name();
        return os;
      }
    }
  }
}

#endif
