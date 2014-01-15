// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/util/signal_handler_manager.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/map.hpp>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace
    {
      boost::mutex GLOBAL_manager_mutex;
      signal_handler_manager* GLOBAL_manager;

      void signal_handler (int sig_num, siginfo_t* info, void* context)
      {
        boost::mutex::scoped_lock const _ (GLOBAL_manager_mutex);
        if (GLOBAL_manager)
        {
          GLOBAL_manager->handle (sig_num, info, context);
        }
      }
    }

    signal_handler_manager::signal_handler_manager()
    {
      boost::mutex::scoped_lock const _ (GLOBAL_manager_mutex);
      assert (!GLOBAL_manager);
      GLOBAL_manager = this;
    }

    signal_handler_manager::~signal_handler_manager()
    {
      boost::mutex::scoped_lock const _ (GLOBAL_manager_mutex);
      assert (GLOBAL_manager);
      GLOBAL_manager = NULL;

      BOOST_FOREACH (int sig_num, _handlers | boost::adaptors::map_keys)
      {
        signal (sig_num, SIG_DFL);
      }
    }

    void signal_handler_manager::add
      (int sig_num, boost::function<void (int, siginfo_t*, void*)> fun)
    {
      boost::mutex::scoped_lock const _ (_handler_mutex);

      if (_handlers.find (sig_num) == _handlers.end())
      {
        struct sigaction sigact;
        sigact.sa_sigaction = signal_handler;
        sigact.sa_flags = SA_RESTART | SA_SIGINFO;

        if (sigaction (sig_num, &sigact, NULL) < 0)
        {
          throw std::runtime_error ("unable to set up signal handler for " + std::string (strsignal (sig_num)));
        }
      }

      _handlers[sig_num].push_back (fun);
    }

    void signal_handler_manager::handle
      (int sig_num, siginfo_t* info, void* context) const
    {
      boost::mutex::scoped_lock const _ (_handler_mutex);
      BOOST_FOREACH ( const boost::function<void (int, siginfo_t*, void*)>& fun
                    , _handlers.find (sig_num)->second
                    )
      {
        fun (sig_num, info, context);
      }
    }
  }
}
