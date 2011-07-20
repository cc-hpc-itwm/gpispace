#include <iostream>

#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/builtin/hello.hpp>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

class HelloImpl : FHG_PLUGIN
                , public example::Hello
{
public:
  HelloImpl () {}
  ~HelloImpl () {}

  FHG_PLUGIN_START()
  {
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  std::string text () const
  {
    return "Hello";
  }
};

EXPORT_FHG_PLUGIN( hello
                 , HelloImpl
                 , "say hello"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v0.0.1"
                 , "GPL"
                 , ""
                 , ""
                 );
