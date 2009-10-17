#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/LoggingConfigurator.hpp>

#include <sdpa/wf/Token.hpp>
#include <sdpa/wf/Parameter.hpp>
#include <sdpa/wf/Activity.hpp>

#include <sdpa/wf/Serialization.hpp>

#include <gwdl/WFSerialization.h>
#include <gwdl/Token.h>

#include <iostream>

struct LogConfig {
  void operator()() {
    sdpa::logging::DefaultConfiguration()();
    std::clog << "private config stuff here." << std::endl;
  }
};

int main(int /* argc */, char ** /* argv */)
{
  using namespace sdpa;
  sdpa::logging::Configurator::configure(LogConfig());
  fhg::log::Configurator::configure();

  SDPA_DEFINE_LOGGER("tests.test-tool");

  SDPA_LOG_ERROR("test");

  sdpa::daemon::Job::ptr_t job(new sdpa::daemon::JobImpl("job-1234", "<job></job>"));
  std::clog << "logging enabled = " << SDPA_ENABLE_LOGGING << std::endl;
  std::clog << "log4cpp = " << SDPA_HAVE_LOG4CPP << std::endl;
  std::clog << "sdpa_logger = " << sdpa_logger.getName() << std::endl;
  std::clog << "id = " << job->id() << std::endl;
  std::clog << "desc = " << job->description() << std::endl;

  sdpa::wf::Activity activity("activity-1"
      , sdpa::wf::Activity::Method("test.so", "loopStep")
      , sdpa::wf::Activity::parameters_t());
  activity.add_parameter(sdpa::wf::Parameter
      ("i", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(42))
      );
  activity.add_parameter(sdpa::wf::Parameter
      ("o", sdpa::wf::Parameter::OUTPUT_EDGE, sdpa::wf::Token(""))
      );
  std::clog << "activity = " << activity << std::endl;

  std::clog << sdpa::wf::Activity::Method("test.so@loopStep") << std::endl;

  try
  {
    gwdl::deserializeWorkflow("foo");
  } catch (const std::runtime_error &re)
  {
    std::clog << "at least it throws now" << std::endl;  
  }

  {
    sdpa::wf::Token iToken((int)42);
    try {
      std::clog << "iToken: " << iToken << std::endl;
      std::clog << "\tas int: " << iToken.data_as<int>() << std::endl;
      std::clog << "\tas float: " << iToken.data_as<float>() << std::endl;
    } catch (const std::exception &ex) {
      std::clog << "\texception: " << ex.what() << std::endl;
    }
  }

  {
    gwdl::Token gtoken(true);
    sdpa::wf::Token stoken(gtoken);
    std::clog << "gtoken: " << gtoken << std::endl;
    std::clog << "stoken: " << stoken << std::endl;;
  }

  {
    gwdl::Token gtoken(false);
    sdpa::wf::Token stoken(gtoken);
    std::clog << "gtoken: " << gtoken << std::endl;;
    std::clog << "stoken: " << stoken << std::endl;;
  }

  {
    std::stringstream sstr;
    boost::archive::text_oarchive oa(sstr);
    {
      sdpa::wf::Token stoken(42);
      stoken.properties().put("foo", "bar");
      oa << stoken;
      LOG(DEBUG, "serialized token " << stoken << " to: " << sstr.str());
    }
    {
      sdpa::wf::Token stoken;
      boost::archive::text_iarchive oa(sstr);
      oa >> stoken;
      LOG(DEBUG, "deserialized token " << stoken << " from: " << sstr.str());
    }
  }

  {
    std::stringstream sstr;
    boost::archive::text_oarchive oa(sstr);
    {
      sdpa::wf::Activity activity("activity-1"
          , sdpa::wf::Activity::Method("test.so", "loopStep")
          , sdpa::wf::Activity::parameters_t());
      activity.add_parameter(sdpa::wf::Parameter
          ("i", sdpa::wf::Parameter::INPUT_EDGE, sdpa::wf::Token(42))
          );
      activity.add_parameter(sdpa::wf::Parameter
          ("o", sdpa::wf::Parameter::OUTPUT_EDGE, sdpa::wf::Token(""))
          );
      oa << activity;
      LOG(DEBUG, "serialized activity " << activity << " to: " << sstr.str());
    }
    {
      sdpa::wf::Activity activity;
      boost::archive::text_iarchive oa(sstr);
      oa >> activity;
      LOG(DEBUG, "deserialized activity " << activity << " from: " << sstr.str());
    }
  }
}
