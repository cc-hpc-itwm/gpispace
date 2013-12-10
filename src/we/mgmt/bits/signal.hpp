// alexander.petry@itwm.fraunhofer.de

#ifndef WE_MGMT_BITS_SIGNAL_HPP
#define WE_MGMT_BITS_SIGNAL_HPP 1

#include <boost/function.hpp>

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
      private:
        typedef typename std::vector<boost::function<F> > funs_type;

      public:
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
          for (typename funs_type::iterator t = targets_.begin(); t != targets_.end(); ++t)
          {
            (*t)();
          }
	}

	template <typename Arg1>
	void operator() (Arg1 a1)
	{
          for (typename funs_type::iterator t = targets_.begin(); t != targets_.end(); ++t)
          {
            (*t)(a1);
          }
	}

	template <typename Arg1, typename Arg2>
	void operator() (Arg1 a1, Arg2 a2)
	{
          for (typename funs_type::iterator t = targets_.begin(); t != targets_.end(); ++t)
          {
            (*t)(a1, a2);
          }
	}

	template <typename Arg1, typename Arg2, typename Arg3>
	void operator() (Arg1 a1, Arg2 a2, Arg3 a3)
	{
          for (typename funs_type::iterator t = targets_.begin(); t != targets_.end(); ++t)
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
