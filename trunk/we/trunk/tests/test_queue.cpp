#include <we/mgmt/bits/queue.hpp>
#include <boost/thread.hpp>

typedef we::mgmt::detail::queue<int, 10> queue_t;

template <typename Q>
static void pusher (Q & q)
{
  for (;;)
  {
    q.put (42);
    std::cerr << "+" << std::flush;
  }
}

int main()
{
  queue_t q0;
  boost::thread thrd (boost::bind (&pusher<queue_t>, boost::ref(q0)));

  sleep (1);

  int val = q0.get();

  std::cerr << "got := " << val << std::endl;

  sleep (1);

  thrd.interrupt();
  thrd.join();

  return EXIT_SUCCESS;
}
