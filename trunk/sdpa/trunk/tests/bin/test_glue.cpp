/*
 * =====================================================================================
 *
 *       Filename:  test_glue.cpp
 *
 *    Description:  test the wf::glue code
 *
 *        Version:  1.0
 *        Created:  11/02/2009 06:15:26 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <fhglog/fhglog.hpp>

#include <sdpa/wf/Token.hpp>
#include <sdpa/wf/Parameter.hpp>
#include <sdpa/wf/Activity.hpp>

#include <sdpa/wf/GwesGlue.hpp>

int main(int argc, char **argv)
{
  fhg::log::Configurator::configure(argc, argv);
  using namespace sdpa;

  {
    gwdl::Token gtoken(true);
    sdpa::wf::Token stoken(sdpa::wf::glue::wrap(gtoken));
    std::clog << "gtoken: " << gtoken << std::endl;
    std::clog << "stoken: " << stoken << std::endl;;
  }

  {
    gwdl::Token gtoken(false);
    sdpa::wf::Token stoken(sdpa::wf::glue::wrap(gtoken));
    std::clog << "gtoken: " << gtoken << std::endl;;
    std::clog << "stoken: " << stoken << std::endl;;
  }

  {
    gwdl::Token gtoken(new gwdl::Data("<data></data>"));
    LOG(DEBUG, "creating empty data token: " << gtoken);
    sdpa::wf::Token stoken(sdpa::wf::glue::wrap(gtoken));
    LOG(DEBUG, "wrapped to: " << stoken);
    stoken.data(42);
    LOG(DEBUG, "updated data: " << stoken);
    gwdl::Token *gtoken2 = sdpa::wf::glue::unwrap(stoken);
    LOG(DEBUG, "unwrapped: " << *gtoken2);
    delete gtoken2;
  }

  {
    try
    {
      gwdl::Properties props;
      props.put("datatype", typeid(int).name());
      gwdl::Data *data(new gwdl::Data("<data>42</data>"));
      gwdl::Token gtoken(props, data);
      sdpa::wf::Token stoken(sdpa::wf::glue::wrap(gtoken));
      std::clog << "gtoken: " << gtoken << std::endl;;
      std::clog << "stoken: " << stoken << std::endl;;
      std::clog << "\tas int: " << stoken.data_as<int>() << std::endl;
    }
    catch (const gwdl::WorkflowFormatException &wfe)
    {
      std::clog << "could not instantiate token-data" << std::endl;
    }
  }

  return 0;
}
