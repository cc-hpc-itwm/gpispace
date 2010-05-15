#include <we/mgmt/bits/queue.hpp>
#include <boost/thread.hpp>

typedef we::mgmt::detail::queue<int> queue_t;

static void pusher (queue_t & q)
{
  for (;;)
  {
    q.put (42);
    std::cerr << "+" << std::flush;
  }
}

int main()
{
  we::mgmt::detail::queue<int> q0 (1024);
  boost::thread thrd (boost::bind (&pusher, boost::ref(q0)));

  sleep (1);

  int val = q0.get();

  std::cerr << "got := " << val << std::endl;

  sleep (1);

  thrd.interrupt();
  thrd.join();

  return EXIT_SUCCESS;
}
