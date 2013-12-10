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
        signal(const std::string & name = "sig_noname")
          : name_(name)
        { }

	template <typename C>
	void connect(C f)
	{
	  targets_.push_back(f);
	}

        bool connected () const
        {
          return ! targets_.empty();
        }

        const std::string &name() const { return name_; }

	void operator() ()
	{
          try
          {
            typedef typename std::vector<boost::function<F> > funcs_t;
            for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
            {
              (*t)();
            }
          }
          catch (const std::exception & ex)
          {
            std::cerr << "E: exception during signal("+name()+"): " << ex.what() << std::endl;
          }
	}

	template <typename Arg1>
	void operator() (Arg1 a1)
	{
          try
          {
            typedef typename std::vector<boost::function<F> > funcs_t;
            for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
            {
              (*t)(a1);
            }
          }
          catch (const std::exception & ex)
          {
            std::cerr << "E: exception during signal("+name()+"): " << ex.what() << std::endl;
          }
	}

	template <typename Arg1, typename Arg2>
	void operator() (Arg1 a1, Arg2 a2)
	{
          try
          {
            typedef typename std::vector<boost::function<F> > funcs_t;
            for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
            {
              (*t)(a1, a2);
            }
          }
          catch (const std::exception & ex)
          {
            std::cerr << "E: exception during signal("+name()+"): " << ex.what() << std::endl;
          }
	}

	template <typename Arg1, typename Arg2, typename Arg3>
	void operator() (Arg1 a1, Arg2 a2, Arg3 a3)
	{
          try
          {
            typedef typename std::vector<boost::function<F> > funcs_t;
            for (typename funcs_t::iterator t = targets_.begin(); t != targets_.end(); ++t)
            {
              (*t)(a1, a2, a3);
            }
          }
          catch (const std::exception & ex)
          {
            std::cerr << "E: exception during signal("+name()+"): " << ex.what() << std::endl;
          }
	}

      private:
        const std::string name_;
	std::vector<boost::function<F> > targets_;
      };
    }
  }
}

#endif
