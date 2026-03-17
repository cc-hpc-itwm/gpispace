#include <gspc/util/syscall/signal_set.hpp>

#include <gspc/util/syscall.hpp>



    namespace gspc::util::syscall
    {
      signal_set::signal_set()
      {
        syscall::sigemptyset (&_);
      }

      signal_set::signal_set (std::initializer_list<int> signals)
        : signal_set()
      {
        for (int const& signal : signals)
        {
          add (signal);
        }
      }

      void signal_set::add (int signal)
      {
        syscall::sigaddset (&_, signal);
      }
    }
