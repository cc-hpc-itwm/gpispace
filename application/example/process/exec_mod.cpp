#include <we/loader/macros.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cmath>

#include <stdexcept>

#include <boost/random.hpp>
#include <limits>

#include "process.hpp"

// ************************************************************************* //

static void exec_wrapper ( gspc::drts::context *
			 , const expr::eval::context & input
			 , expr::eval::context & output
			 )
{
  const std::string& command (boost::get<std::string> (input.value ("command")));

  MLOG (INFO, "exec:  = \"" << command << "\"");

  long ec = process::execute (command, 0, 0, 0, 0);

  MLOG (INFO, "process returned with: " << ec);
  output.bind ("ec", pnet::type::value::value_type (ec));
}

// ************************************************************************* //

static void selftest ( gspc::drts::context *
		     , const expr::eval::context &
		     , expr::eval::context &
		     )
{
  const std::size_t num_bytes = 274176000;

  char * buf (new char[num_bytes]);

  if (!buf)
  {
    throw std::runtime_error ("BUMMER! Not enough memory for exec.selftest!");
  }

  std::ifstream ifs ("/fhgfs/HPC/rahn/kdm/simple.app10.su");
  ifs.read (buf, num_bytes);

  std::size_t bytes_read
    (process::execute ( "sustack"
                      , buf, num_bytes
                      , buf, num_bytes
                      )
    );

  std::ofstream ofs ("/fhgfs/HPC/rahn/kdm/simple.app10.su.stacked");
  ofs.write (buf, bytes_read);

  MLOG(INFO, "selftest: bytes_read = " << bytes_read);
}

// ************************************************************************* //

WE_MOD_INITIALIZE_START (exec);
{
  WE_REGISTER_FUN_AS (exec_wrapper, "exec");
  WE_REGISTER_FUN (selftest);
}
WE_MOD_INITIALIZE_END (exec);

WE_MOD_FINALIZE_START (exec);
{
}
WE_MOD_FINALIZE_END (exec);
