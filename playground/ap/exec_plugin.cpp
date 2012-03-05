#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>

#include <string>

#include <process/process.hpp>

class ExecPluginImpl : FHG_PLUGIN
{
public:
  virtual ~ExecPluginImpl() {}

  FHG_PLUGIN_START()
  {
    std::string command =
      fhg_kernel()->get("command", "/bin/sleep 1");

    if (0 == execute(command))
    {
      fhg_kernel()->shutdown();
      FHG_PLUGIN_STARTED();
    }
    else
    {
      FHG_PLUGIN_FAILED(EFAULT);
    }
  }

  FHG_PLUGIN_STOP()
  {
    FHG_PLUGIN_STOPPED();
  }

  int execute (std::string const & command)
  {
    process::circular_buffer buf_std_err;
    char buf_std_out[1 << 10];

    process::execute_return_type exec_result;

    LOG (INFO, "running command " << command);

    memset (buf_std_out, 0, sizeof (buf_std_out));
    buf_std_err.clear();

    exec_result =
      process::execute ( command
                       , process::const_buffer (0, 0)
                       , process::buffer (buf_std_out, sizeof(buf_std_out)-1)
                       , buf_std_err
                       , process::file_const_buffer_list()
                       , process::file_buffer_list()
                       )
      ;

    // sanitize output
    buf_std_out[sizeof(buf_std_out)-1] = '\0';
    if (! buf_std_err.empty())
    {
      buf_std_err[buf_std_err.size()-1] = '\0';
    }

    LOG(INFO, "command finished: " << exec_result.exit_code);

    std::cout << buf_std_out << std::endl;
    std::cerr << &buf_std_err[0] << std::endl;

    return exec_result.exit_code;
  }
};

EXPORT_FHG_PLUGIN( exec_plugin
                 , ExecPluginImpl
                 , "provides access to process::exec"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , ""
                 , ""
                 );
