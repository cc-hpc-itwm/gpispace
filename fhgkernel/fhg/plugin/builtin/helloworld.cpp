#include <signal.h>

#include <iostream>

#include <fhglog/minimal.hpp>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/builtin/stats.hpp>
#include <fhg/plugin/builtin/hello.hpp>
#include <fhg/plugin/builtin/world.hpp>
#include <fhg/plugin/builtin/helloworld.hpp>

#include <boost/thread.hpp>

static stats::Statistics *statistics = 0;

static void (*count)(const char*name) = 0;

static void real_count(const char *name)
{
  statistics->inc(name);
}

static void start_timer(const char *name)
{
  if (statistics) statistics->start_timer(name);
}
static void stop_timer(const char *name)
{
  if (statistics) statistics->stop_timer(name);
}

static void fake_count(const char*) {}

class HelloWorldImpl : FHG_PLUGIN
                     , public example::HelloWorld
{
public:
  HelloWorldImpl ()
    : m_hello (0)
    , m_world(0)
  {}

  FHG_PLUGIN_START()
  {
    m_hello = fhg_kernel()->acquire<example::Hello>("hello");
    m_world = fhg_kernel()->acquire<example::World>("world");

    m_num_iterations = fhg_kernel ()->get<size_t> ("num_iter", 2);

    if ((statistics = fhg_kernel()->acquire<stats::Statistics>("stats")) != 0)
    {
      count = real_count;
    }
    else
    {
      count = fake_count;
    }

    fhg_kernel()->schedule( "say"
                          , boost::bind( &HelloWorldImpl::do_it
                                       , this
                                       )
                          );
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    fhg_kernel()->release("world");
    fhg_kernel()->release("hello");
    fhg_kernel()->release("stats");
    MLOG(TRACE, "helloworld plugin stopped");
    FHG_PLUGIN_STOPPED();
  }

  void do_it()
  {
    start_timer("helloworld.say");
    say (std::cout);
    stop_timer("helloworld.say");

    if (m_num_iterations --> 0)
    {
      fhg_kernel()->schedule( "say"
                            , boost::bind( &HelloWorldImpl::do_it
                                         , this
                                         )
                            , 1
                            );
    }
  }

  void say (std::ostream &os) const
  {
    count("helloworld.say");
    os << m_hello->text() << ", " << m_world->text() << "!" << std::endl;
  }

  bool is_done () const
  {
    return 0 == m_num_iterations;
  }
private:
  example::Hello *m_hello;
  example::World *m_world;

  size_t m_num_iterations;
};

EXPORT_FHG_PLUGIN( helloworld
                 , HelloWorldImpl
                 , ""
                 , "say world"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v0.0.1"
                 , "GPL"
                 , "hello,world"
                 , ""
                 );
