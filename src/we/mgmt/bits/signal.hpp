// alexander.petry@itwm.fraunhofer.de

#ifndef WE_MGMT_BITS_SIGNAL_HPP
#define WE_MGMT_BITS_SIGNAL_HPP 1

#include <boost/function.hpp>

#include <iostream>
#include <vector>

namespace we
{
  namespace mgmt
  {
    namespace util
    {
      template <typename F = void (void)>
      class signal
      {
      public:
        explicit
        signal(const std::string&)
        {}

	template <typename C>
	void connect(C f)
	{
	  targets_.push_back(f);
	}

        bool connected () const
        {
          return ! targets_.empty();
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
    }
  }
}

#endif
