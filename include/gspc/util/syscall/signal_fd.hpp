#pragma once

#include <gspc/util/syscall/signal_set.hpp>



    namespace gspc::util::syscall
    {
      struct signal_fd
      {
        signal_fd (signal_set const&);
        ~signal_fd();

        signal_fd (signal_fd const&) = delete;
        signal_fd (signal_fd&&) = delete;
        signal_fd& operator= (signal_fd const&) = delete;
        signal_fd& operator= (signal_fd&&) = delete;

        int _;
      };
    }
