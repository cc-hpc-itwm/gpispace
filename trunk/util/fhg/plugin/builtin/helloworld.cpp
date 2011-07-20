#include <iostream>

#include <fhglog/minimal.hpp>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/builtin/stats.hpp>
#include <fhg/plugin/builtin/hello.hpp>
#include <fhg/plugin/builtin/world.hpp>
#include <fhg/plugin/builtin/helloworld.hpp>

#include <boost/thread.hpp>

static fhg::plugin::Kernel *krnl = 0;
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
  ~HelloWorldImpl () {}

  FHG_PLUGIN_START(kernel)
  {
    krnl = kernel;

    m_hello = kernel->acquire<example::Hello>("hello");
    m_world = kernel->acquire<example::World>("world");

    if ((statistics = kernel->acquire<stats::Statistics>("stats")) != 0)
    {
      count = real_count;
    }
    else
    {
      count = fake_count;
    }

    krnl->schedule( boost::bind( &HelloWorldImpl::do_it
                               , this
                               )
                  );
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP(kernel)
  {
    kernel->release("world");
    kernel->release("hello");
    kernel->release("stats");
    MLOG(INFO, "helloworld plugin stopped");
    FHG_PLUGIN_STOPPED();
  }

  void do_it()
  {
    start_timer("helloworld.say");
    say (std::cout);

    krnl->schedule( boost::bind( &HelloWorldImpl::do_it
                               , this
                               )
                  , 5
                  );

    count("helloworld.say");
    stop_timer("helloworld.say");
  }

  void say (std::ostream &os) const
  {
    os << m_hello->text() << ", " << m_world->text() << std::endl;
  }

private:
  example::Hello *m_hello;
  example::World *m_world;
};

EXPORT_FHG_PLUGIN( helloworld
                 , HelloWorldImpl
                 , "say world"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v0.0.1"
                 , "GPL"
                 , "hello,world"
                 , ""
                 );
