#pragma once

#include <initializer_list>

#include <signal.h>



    namespace gspc::util::syscall
    {
      struct signal_set
      {
        signal_set();
        signal_set (std::initializer_list<int> /* signals */);
        void add (int signal);

        sigset_t _;
      };
    }
