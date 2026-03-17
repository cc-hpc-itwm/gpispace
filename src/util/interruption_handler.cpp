#include <gspc/util/interruption_handler.hpp>
#include <gspc/util/syscall.hpp>

#include <stdexcept>

#include <sys/signalfd.h>


  namespace gspc::util
  {
    void interruption_handler::wait_for_finish_or_interruption
      (std::function<void()> const& on_interruption) const
    {
      signalfd_siginfo fdsi;

      do
      {
        if ( gspc::util::syscall::read (_fd._, &fdsi, sizeof (fdsi))
           != sizeof (fdsi)
           )
        {
          throw std::logic_error ("signalfd read with invalid size");
        }

        if (fdsi.ssi_signo == SIGINT)
        {
          on_interruption();
        }
      } while (fdsi.ssi_signo != SIGUSR2);
    }

    void interruption_handler::finished_on_scope_exit_function()
    {
      gspc::util::syscall::kill (gspc::util::syscall::getpid(), SIGUSR2);
    }
  }
