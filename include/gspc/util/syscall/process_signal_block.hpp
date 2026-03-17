#pragma once

#include <gspc/util/syscall/signal_set.hpp>



    namespace gspc::util::syscall
    {
      struct process_signal_block
      {
        process_signal_block (signal_set const&);
        ~process_signal_block();

        process_signal_block (process_signal_block const&) = delete;
        process_signal_block (process_signal_block&&) = delete;
        process_signal_block& operator= (process_signal_block const&) = delete;
        process_signal_block& operator= (process_signal_block&&) = delete;

      private:
        signal_set _old_set;
      };
    }
