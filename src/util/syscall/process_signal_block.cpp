#include <gspc/util/syscall/process_signal_block.hpp>

#include <gspc/util/syscall.hpp>



    namespace gspc::util::syscall
    {
      process_signal_block::process_signal_block (signal_set const& set)
      {
        syscall::sigprocmask (SIG_BLOCK, &set._, &_old_set._);
      }
      process_signal_block::~process_signal_block()
      {
        syscall::sigprocmask (SIG_SETMASK, &_old_set._, nullptr);
      }
    }
