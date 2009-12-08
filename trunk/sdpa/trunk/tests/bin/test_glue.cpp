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

#include <gwdl/XMLUtils.h>
#include <gwdl/Libxml2Builder.h>

#include <fstream>

int main(int , char **)
{
  fhg::log::Configurator::configure();
  using namespace sdpa;

  {
    gwdl::Token gtoken(gwdl::Token::CONTROL_TRUE);
    sdpa::wf::Token stoken(sdpa::wf::glue::wrap(gtoken));
//    std::clog << "gtoken: " << gtoken << std::endl;
    std::clog << "stoken: " << stoken << std::endl;;
  }

  {
    gwdl::Token gtoken(gwdl::Token::CONTROL_FALSE);
    sdpa::wf::Token stoken(sdpa::wf::glue::wrap(gtoken));
//    std::clog << "gtoken: " << gtoken << std::endl;;
    std::clog << "stoken: " << stoken << std::endl;;
  }

  {
    gwdl::Token gtoken(gwdl::Data::ptr_t(new gwdl::Data("text")));
//    LOG(DEBUG, "creating empty data token: " << gtoken);
    sdpa::wf::Token stoken(sdpa::wf::glue::wrap(gtoken));
    LOG(DEBUG, "wrapped to: " << stoken);
    stoken.data(42);
    LOG(DEBUG, "updated data: " << stoken);
    gwdl::Token *gtoken2 = sdpa::wf::glue::unwrap(stoken);
//    LOG(DEBUG, "unwrapped: " << *gtoken2);
    delete gtoken2;
  }

  {
    try
    {
      gwdl::Properties::ptr_t props(new gwdl::Properties());
      props->put("datatype", typeid(int).name());
      gwdl::Data::ptr_t data(new gwdl::Data("42"));
      gwdl::Token gtoken(props, data);
      sdpa::wf::Token stoken(sdpa::wf::glue::wrap(gtoken));
//      std::clog << "gtoken: " << gtoken << std::endl;;
      std::clog << "stoken: " << stoken << std::endl;;
      std::clog << "\tas int: " << stoken.data_as<int>() << std::endl;
    }
    catch (const gwdl::WorkflowFormatException &wfe)
    {
      std::clog << "could not instantiate token-data" << std::endl;
    }
  }

  {
    std::string path_to_desc("../sdpa/workflows/workflow-result-test.gwdl");
    try
    {
      using namespace sdpa::wf::glue;

	  gwdl::Libxml2Builder builder;
	  gwdl::Workflow::ptr_t gwdl_workflow(builder.deserializeWorkflowFromFile(path_to_desc));
      gwdl::workflow_result_t wf_result = gwdl_workflow->getResults();
	  if (wf_result.empty())
	  {
		std::clog << "Failed: no result tokens found!"<< std::endl;
		return 1;
	  }
      sdpa::job_result_t result = wrap(wf_result);
	  if (result.empty())
	  {
		std::clog << "Failed: no result tokens wrapped!" << std::endl;
	  }
      gwdl::deallocate_workflow_result(wf_result);

      for (sdpa::job_result_t::const_iterator r(result.begin()); r != result.end(); ++r)
      {
        std::clog << "tokens on place " << r->first << ":" << std::endl;
        for (token_list_t::const_iterator token(r->second.begin()); token != r->second.end(); ++token)
        {
          std::clog << "\t" << *token << std::endl;
        }
      }
    }
    catch (const std::exception &ex)
    {
      std::clog << "could not parse workflow: " << ex.what() << std::endl;
	  return 2;
    }
  }

  return 0;
}
