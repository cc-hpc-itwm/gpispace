// alexander.petry@itwm.fraunhofer.de

#include <sstream> // ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/StreamAppender.hpp>
#include <fhglog/SynchronizedAppender.hpp>
#include <unistd.h> // usleep
#include <pthread.h>

struct t_param
{
  char id;
  std::size_t loop_count;
};

void *thread(void *arg)
{
  using namespace fhg::log;
  t_param *p = static_cast<t_param*>(arg);
  LOG(DEBUG, "t" << p->id << " started");
#ifdef __APPLE__
  pthread_yield_np(); // apple only allows you to force yielding.
#else
  pthread_yield();
#endif
  for (std::size_t i(0); i < p->loop_count; ++i)
  {
    logger_t log(getLogger(std::string("t") + p->id));
    LOG_DEBUG(log, "loop count " << i);
    if (0 == (i % 50))
    {
      LOG(DEBUG, "t" << p->id << " sleeping");
      usleep(100);
    }
  }
  LOG(DEBUG, "t" << p->id << " finished");
  return arg;
}

static std::ostringstream logstream;

int main (int , char **)
{
  using namespace fhg::log;

  int errcount(0);
  logger_t root(getLogger());

  root.addAppender(Appender::ptr_t(new SynchronizedAppender(new StreamAppender("stringstream", logstream, "%m"))));

  {
    std::clog << "** testing multiple threads...";
    std::size_t loop_count = 1000;

    struct t_param pt0 = { '0', loop_count };
    struct t_param pt1 = { '1', loop_count };
    struct t_param pt2 = { '2', loop_count };

    pthread_t t0;
    pthread_t t1;
    pthread_t t2;

    LOG(DEBUG, "starting threads...");
    pthread_create(&t0, NULL, &thread, &pt0);
    pthread_create(&t1, NULL, &thread, &pt1);
    pthread_create(&t2, NULL, &thread, &pt2);

    LOG(DEBUG, "joining...");
    pthread_join(t2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t0, NULL);

    // didn't segfault...
    std::clog << "OK!" << std::endl;
  }

  return errcount;
}
