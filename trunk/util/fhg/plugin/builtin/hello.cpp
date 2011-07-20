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

  FHG_PLUGIN_START(config)
  {
    m_thread = boost::thread(boost::bind( &HelloImpl::say
                                        , this
                                        )
                            );
    return 0;
  }

  FHG_PLUGIN_STOP()
  {
    m_thread.join();
    return 0;
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
