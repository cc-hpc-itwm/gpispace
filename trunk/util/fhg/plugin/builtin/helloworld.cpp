#include <iostream>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/builtin/hello.hpp>
#include <fhg/plugin/builtin/world.hpp>
#include <fhg/plugin/builtin/helloworld.hpp>

#include <boost/thread.hpp>

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
    m_hello = kernel->acquire_plugin<example::Hello>("hello");
    m_world = kernel->acquire_plugin<example::World>("world");

    m_thread = boost::thread( &HelloWorldImpl::say
                            , this
                            , boost::ref(std::cout)
                            );
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP(kernel)
  {
    m_thread.join();
    kernel->release("world");
    kernel->release("hello");
    FHG_PLUGIN_STOPPED();
  }

  void say (std::ostream &os) const
  {
    os << m_hello->text() << ", " << m_world->text() << std::endl;
  }

private:
  example::Hello *m_hello;
  example::World *m_world;
  boost::thread m_thread;
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
