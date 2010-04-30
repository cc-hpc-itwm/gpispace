#include <sstream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/we.hpp>
#include "simple_generator.hpp"

// specific
#include "kdm_simple.hpp"

// generic
#include "module.hpp"
#include "context.hpp"

int main (int argc, char ** argv)
{
  we::transition_t simple_trans (kdm::kdm<we::activity_t>::generate());

  we::activity_t act ( simple_trans );

  act.input().push_back
    ( we::input_t::value_type
      ( we::token_t ( "config_file"
                , literal::STRING
                , std::string ((argc > 1) ? argv[1] : "/scratch/KDM.conf")
                )
      , simple_trans.input_port_by_name ("config_file")
      )
    );

  // dump activity for test purposes
  {
    std::ofstream ofs ("kdm_simple.pnet");
    ofs << we::util::text_codec::encode (act);
  }

  struct exec_context ctxt;
  act.execute (ctxt);

  we::mgmt::type::detail::printer<we::activity_t, std::ostream> printer (act, std::cout);
  printer << "output := "
          << act.output()
          << std::endl;

  if ( act.output().empty() )
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
