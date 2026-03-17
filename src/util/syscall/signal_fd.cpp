#include <gspc/util/syscall/signal_fd.hpp>

#include <gspc/util/syscall.hpp>



    namespace gspc::util::syscall
    {
      signal_fd::signal_fd (signal_set const& set)
        : _ (syscall::signalfd (-1, &set._, 0))
      {}
      signal_fd::~signal_fd()
      {
        syscall::close (_);
      }
    }
