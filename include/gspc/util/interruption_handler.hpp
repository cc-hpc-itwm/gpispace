#pragma once

#include <functional>

#include <gspc/util/finally.hpp>
#include <gspc/util/syscall/process_signal_block.hpp>
#include <gspc/util/syscall/signal_fd.hpp>
#include <gspc/util/syscall/signal_set.hpp>


  namespace gspc::util
  {
    //! \note construct before starting ANY thread, e.g. as the very
    //! first command in main(), or else a random thread may receive
    //! the signal
    class interruption_handler
    {
    public:
      interruption_handler()
        : _mask ({SIGINT, SIGUSR2})
        , _block (_mask)
        , _fd (_mask)
      {}

      void wait_for_finish_or_interruption
        (std::function<void()> const& on_interruption) const;

    private:
      static void finished_on_scope_exit_function();

    public:
      decltype (finally (&interruption_handler::finished_on_scope_exit_function))
        notifier() const
      {
        return finally (&interruption_handler::finished_on_scope_exit_function);
      }

    private:
      gspc::util::syscall::signal_set _mask;
      gspc::util::syscall::process_signal_block _block;
      gspc::util::syscall::signal_fd _fd;
    };
  }
