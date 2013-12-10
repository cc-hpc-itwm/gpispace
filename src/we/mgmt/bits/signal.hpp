// alexander.petry@itwm.fraunhofer.de

#ifndef WE_MGMT_BITS_SIGNAL_HPP
#define WE_MGMT_BITS_SIGNAL_HPP 1

#include <boost/foreach.hpp>
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
          BOOST_FOREACH (boost::function<F> const& f, targets_)
          {
            f();
          }
	}

	template <typename Arg1>
	void operator() (Arg1 a1)
	{
          BOOST_FOREACH (boost::function<F> const& f, targets_)
          {
            f(a1);
          }
	}

	template <typename Arg1, typename Arg2>
	void operator() (Arg1 a1, Arg2 a2)
	{
          BOOST_FOREACH (boost::function<F> const& f, targets_)
          {
            f(a1, a2);
          }
	}

	template <typename Arg1, typename Arg2, typename Arg3>
	void operator() (Arg1 a1, Arg2 a2, Arg3 a3)
	{
          BOOST_FOREACH (boost::function<F> const& f, targets_)
          {
            f(a1, a2, a3);
          }
	}

      private:
	std::vector<boost::function<F> > targets_;
      };
    }
  }
}

#endif
