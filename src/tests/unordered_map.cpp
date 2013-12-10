#include <iostream>
#include <fhglog/fhglog.hpp>
#include <boost/unordered_map.hpp>
#include <boost/lexical_cast.hpp>

#ifndef __APPLE__
#include <malloc.h>

static void log_malloc_info ()
{
  struct mallinfo info = mallinfo();
#define p(x) LOG(DEBUG, "   " #x << " = " << info. x)
  p(arena);
  p(ordblks);
  p(smblks);
  p(hblks);
  p(hblkhd);
  p(usmblks);
  p(fsmblks);
  p(uordblks);
  p(fordblks);
  p(keepcost);
#undef p
}
#else
static void log_malloc_info() { }
#endif

int main (int ac, char *av[])
{
  FHGLOG_SETUP();

  std::size_t cnt (1000 * 1000);
  if (ac > 1)
    cnt = boost::lexical_cast<std::size_t>(av[1]);

  typedef boost::unordered_map<std::string, unsigned long long> map_t;

  LOG(INFO, "inserting " << cnt << " number of items...");

  map_t m;

  LOG(DEBUG, "***** map statistics (before)");
  LOG(DEBUG, "        #buckets = " << m.bucket_count());
  LOG(DEBUG, "     #maxbuckets = " << m.max_bucket_count());
  LOG(DEBUG, "     load_factor = " << m.load_factor());
  LOG(DEBUG, " max_load_factor = " << m.max_load_factor());

  log_malloc_info ();

  for (std::size_t i (0); i < cnt; ++i)
  {
    m[ boost::lexical_cast<std::string>(i) ] = i;
  }

  LOG(DEBUG, "***** map statistics (after)");
  LOG(DEBUG, "        #buckets = " << m.bucket_count());
  LOG(DEBUG, "     #maxbuckets = " << m.max_bucket_count());
  LOG(DEBUG, "     load_factor = " << m.load_factor());
  LOG(DEBUG, " max_load_factor = " << m.max_load_factor());

  log_malloc_info ();

  m.clear();

  LOG(DEBUG, "***** map statistics (clear)");
  LOG(DEBUG, "        #buckets = " << m.bucket_count());
  LOG(DEBUG, "     #maxbuckets = " << m.max_bucket_count());
  LOG(DEBUG, "     load_factor = " << m.load_factor());
  LOG(DEBUG, " max_load_factor = " << m.max_load_factor());

  log_malloc_info ();

  m.rehash (11);

  LOG(DEBUG, "***** map statistics (rehash)");
  LOG(DEBUG, "        #buckets = " << m.bucket_count());
  LOG(DEBUG, "     #maxbuckets = " << m.max_bucket_count());
  LOG(DEBUG, "     load_factor = " << m.load_factor());
  LOG(DEBUG, " max_load_factor = " << m.max_load_factor());

  log_malloc_info ();
}
