#include <iostream>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/builtin/hello.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

class HelloImpl : IS_A_FHG_PLUGIN
                , public hello::Hello
{
public:
  HelloImpl () {}
  ~HelloImpl () {}

  FHG_PLUGIN_START(kernel)
  {
    m_thread = boost::thread(boost::bind( &HelloImpl::say
                                        , this
                                        )
                            );
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP(kernel)
  {
    m_thread.join();
    FHG_PLUGIN_STOPPED();
  }

  void say () const
  {
    std::cout << "Hello" << std::endl;
  }

private:
  boost::thread m_thread;
};

FHG_PLUGIN( hello
          , HelloImpl
          , "say hello"
          , "Alexander Petry <petry@itwm.fhg.de>"
          , "v0.0.1"
          , "GPL"
          , ""
          , ""
          );
