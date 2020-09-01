#pragma once

#include <functional>

#include <mutex>
#include <list>
#include <map>

#include <signal.h>

namespace fhg
{
  namespace iml
{
  namespace util
  {
    struct signal_handler_manager
    {
      signal_handler_manager();
      ~signal_handler_manager();

      void handle (int sig_num, siginfo_t* info, void* context) const;

    private:
      friend struct scoped_signal_handler;

      mutable std::mutex _handler_mutex;
      using functions = std::list<std::function<void (int, siginfo_t*, void*)>>;
      std::map<int, std::pair<struct sigaction, functions>> _handlers;
    };

    struct scoped_signal_handler
    {
      scoped_signal_handler ( signal_handler_manager&
                            , int sig_num
                            , std::function<void (int, siginfo_t*, void*)>
                            );
      ~scoped_signal_handler();

    private:
      signal_handler_manager& _manager;
      int _sig_num;
      signal_handler_manager::functions::iterator _it;
    };
    }
  }
}
