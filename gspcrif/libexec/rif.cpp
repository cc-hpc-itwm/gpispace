#include "rif.hpp"

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <boost/foreach.hpp>

#include <gspc/net.hpp>
#include <gspc/rif.hpp>

class RifImpl;
static RifImpl *s_rif = 0;

static void s_handle_rif ( std::string const &dst
                         , gspc::net::frame const &rqst
                         , gspc::net::user_ptr user
                         )
{
  gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

  std::string cmd = rqst.get_body_as_string ();

  std::vector<std::string> argv;
  gspc::rif::parse ( rqst.get_body_as_string ()
                   , argv
                   );

  for (size_t i = 0 ; i < argv.size () ; ++i)
  {
    std::stringstream sstr;
    sstr << "argv[" << i << "] = '" << argv [i] << "'" << std::endl;
    rply.add_body (sstr.str ());
  }

  user->deliver (rply);
}

class RifImpl : FHG_PLUGIN
{
public:
  RifImpl ()
  {
    s_rif = this;
  }

  ~RifImpl ()
  {
    s_rif = 0;
  }

  FHG_PLUGIN_START()
  {
    gspc::net::handle
      ("/service/rif", &s_handle_rif);
    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }
};

EXPORT_FHG_PLUGIN( rif
                 , RifImpl
                 , "rif"
                 , "provides access to the rif infrastructure"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
