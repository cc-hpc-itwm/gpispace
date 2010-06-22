#include <sstream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <we/we.hpp>
#include "complex_generator.hpp"

// specific
// generic
#include "module.hpp"
#include "context.hpp"

int main (int argc, char ** argv)
{
  we::transition_t kdm_complex (kdm::kdm<we::activity_t>::generate());

  we::activity_t act (kdm_complex);

  act.add_input
    ( we::input_t::value_type
      ( we::token_t ( "config_file"
                , literal::STRING
                , std::string ((argc > 1) ? argv[1] : "/scratch/KDM.conf")
                )
      , kdm_complex.input_port_by_name ("config_file")
      )
    );

  // dump activity for test purposes
  {
    std::ofstream ofs ("kdm_complex.pnet");
    ofs << we::util::text_codec::encode (act);
  }

  we::loader::loader loader;
  loader.append_search_path
    (std::string ((argc> 2) ? argv[2] : "/amd/nfs/root/gpfs/u/r/rahn/SDPA/trunk/we/trunk/build/kdm/mod/"));
  struct exec_context ctxt (loader);
  act.execute (ctxt);

  we::mgmt::type::detail::printer<we::activity_t, std::ostream>
    printer (act, std::cout);

  printer << "output := "
          << act.output()
          << std::endl;

  std::cout << act << std::endl;

  if ( act.output().empty() )
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
