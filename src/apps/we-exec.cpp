#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/mgmt/context.hpp>
#include <we/mgmt/layer.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>

#include <fhg/error_codes.hpp>
#include <fhg/revision.hpp>
#include <fhg/util/thread/queue.hpp>

#include <fhglog/fhglog.hpp>

#include <sdpa/job_states.hpp>

#include <boost/bind.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <list>
#include <vector>

#include <sysexits.h>

namespace
{
  struct context : public we::mgmt::context
  {
    context ( const we::mgmt::layer::id_type& id
            , we::loader::loader* loader
            , we::mgmt::layer* layer
            , boost::function<we::mgmt::layer::id_type
                               (we::mgmt::layer::id_type const&)
                             > const& new_id
            )
      : _id (id)
      , _loader (loader)
      , _layer (layer)
      , _new_id (new_id)
    {}

    virtual int handle_internally (we::mgmt::type::activity_t& act, net_t& n)
    {
      return handle_externally (act, n);
    }
    virtual int handle_internally (we::mgmt::type::activity_t&, mod_t&)
    {
      throw std::runtime_error ("NO internal mod here!");
    }
    virtual int handle_internally (we::mgmt::type::activity_t&, expr_t&)
    {
      throw std::runtime_error ("NO internal expr here!");
    }
    virtual int handle_externally (we::mgmt::type::activity_t& act, net_t&)
    {
      _layer->submit (_new_id (_id),  act, we::type::user_data());
      return 0;
    }
    virtual int handle_externally (we::mgmt::type::activity_t& act, mod_t& mod)
    {
      try
      {
        //!\todo pass a real gspc::drts::context
        we::loader::module_call (*_loader, 0, act, mod);
        _layer->finished (_id, act);
      }
      catch (std::exception const& ex)
      {
        std::cerr << "handle-ext(" << act.transition().name() << ") failed: " << ex.what() << std::endl;

        _layer->failed ( _id
                       , act
                       , fhg::error::MODULE_CALL_FAILED
                       , ex.what()
                       );
      }
      return 0;
    }
    virtual int handle_externally (we::mgmt::type::activity_t&, expr_t&)
    {
      throw std::runtime_error ("NO external expr here!");
    }


  private:
    we::mgmt::layer::id_type _id;
    we::loader::loader* _loader;
    we::mgmt::layer* _layer;
    boost::function<we::mgmt::layer::id_type (we::mgmt::layer::id_type const&)>
      _new_id;
  };

  struct job_t
  {
    job_t()
    {}

    job_t (const we::mgmt::layer::id_type& id_, const std::string& desc_)
      : id (id_)
      , desc (desc_)
    {}

    we::mgmt::layer::id_type id;
    std::string desc;
  };

  struct sdpa_daemon
  {
    typedef boost::unordered_map< we::mgmt::layer::id_type
                                , we::mgmt::layer::id_type
                                > id_map_t;

    sdpa_daemon ( std::size_t num_worker
                , we::loader::loader* loader
                , we::mgmt::type::activity_t const act
                , boost::optional<std::size_t> const timeout
                )
        : _mutex_id()
        , _id (0)
        , mgmt_layer_ (this, boost::bind (&sdpa_daemon::gen_id, this))
        , _mutex_id_map()
        , id_map_()
        , jobs_()
        , worker_()
        , _loader (loader)
        , _job_status (sdpa::status::RUNNING)
        , _result()
        , _job_id (gen_id())
        , _timeout_thread
          (boost::bind (&sdpa_daemon::maybe_cancel_after_ms, this, timeout))
    {
      for (std::size_t n (0); n < num_worker; ++n)
      {
        worker_.push_back
          (new boost::thread (boost::bind (&sdpa_daemon::worker, this, n)));
      }

      mgmt_layer_.submit (_job_id, act, we::type::user_data());
    }

    ~sdpa_daemon()
    {
      BOOST_FOREACH (boost::thread* t, worker_)
      {
        t->interrupt();
        t->join();
        delete t;
      }

      _timeout_thread.interrupt();
      if (_timeout_thread.joinable())
      {
        _timeout_thread.join();
      }
    }

    void worker (const std::size_t rank)
    {
      MLOG (INFO, "SDPA layer worker-" << rank << " started");

      for (;;)
      {
        job_t const job (jobs_.get());

        context ctxt ( job.id
                     , _loader
                     , &mgmt_layer_
                     , boost::bind (&sdpa_daemon::add_mapping, this, _1)
                     );

        we::mgmt::type::activity_t (job.desc).execute (&ctxt);
      }

      MLOG (INFO, "SDPA layer worker-" << rank << " stopped");
    }

    we::mgmt::layer::id_type gen_id()
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex_id);

      return boost::lexical_cast<std::string> (++_id);
    }

    we::mgmt::layer::id_type
      add_mapping (const we::mgmt::layer::id_type& old_id)
    {
      we::mgmt::layer::id_type const new_id (gen_id());

      boost::unique_lock<boost::recursive_mutex> const _ (_mutex_id_map);

      id_map_[new_id] = old_id;

      return new_id;
    }
    boost::optional<we::mgmt::layer::id_type>
      get_and_delete_mapping (const we::mgmt::layer::id_type& id)
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex_id_map);

      boost::optional<we::mgmt::layer::id_type> mapped;

      id_map_t::iterator pos (id_map_.find (id));

      if (pos != id_map_.end())
      {
        mapped = pos->second;

        id_map_.erase (pos);
      }

      return mapped;
    }

    sdpa::status::code wait_while_job_is_running() const
    {
      boost::unique_lock<boost::mutex> _ (_mutex_job_status);

      while (sdpa::status::is_running (_job_status))
      {
        _condition_job_status_changed.wait (_);
      }

      return _job_status;
    }

    std::string const& result() const
    {
      if (not _result)
      {
        throw std::runtime_error ("missing result");
      }

      return *_result;
    }

    void submit ( const we::mgmt::layer::id_type& id
                , const std::string& desc
                , std::list<we::type::requirement_t> const&
                , const we::type::schedule_data&
                , const we::type::user_data&
                )
    {
      jobs_.put (job_t (id, desc));
    }

    void cancel (const we::mgmt::layer::id_type& id, const std::string& desc)
    {
      std::cout << "cancel[" << id << "] = " << desc << std::endl;

      boost::optional<we::mgmt::layer::id_type> const mapped
        (get_and_delete_mapping (id));

      if (mapped)
      {
        mgmt_layer_.canceled (*mapped);
      }
    }

    void finished (const we::mgmt::layer::id_type& id, const std::string& desc)
    {
      boost::optional<we::mgmt::layer::id_type> const mapped
        (get_and_delete_mapping (id));

      we::mgmt::type::activity_t const act (desc);

      if (mapped)
      {
        mgmt_layer_.finished (*mapped, act);
      }
      else if (id == _job_id)
      {
        std::cout << "finished [" << id << "]" << std::endl;
        BOOST_FOREACH ( const we::mgmt::type::activity_t::token_on_port_t& top
                      , act.output()
                      )
        {
          std::cout << act.transition().get_port (top.second).name()
                    << " => " << pnet::type::value::show (top.first) << std::endl;
        }

        _result = desc;
        set_job_status (sdpa::status::FINISHED);
      }
      else
      {
        throw std::invalid_argument ("finished(" + id + "): no such id");
      }
    }

    void failed ( const we::mgmt::layer::id_type& id
                , const std::string& desc
                , const int error_code
                , const std::string& reason
                )
    {
      boost::optional<we::mgmt::layer::id_type> const mapped
        (get_and_delete_mapping (id));

      we::mgmt::type::activity_t const act (desc);

      if (mapped)
      {
        mgmt_layer_.failed (*mapped, act, error_code, reason);
      }
      else if (id == _job_id)
      {
        std::cout << "failed [" << id << "] = ";
        std::cout << " error-code := " << error_code
                  << " reason := " << reason
                  << " activity := " << act.transition().name()
                  << std::endl;
        _result = desc;
        set_job_status (sdpa::status::FAILED);
      }
      else
      {
        throw std::invalid_argument ("failed(" + id + "): no such id");
      }
    }

    void canceled (const we::mgmt::layer::id_type& id)
    {
      boost::optional<we::mgmt::layer::id_type> const mapped
        (get_and_delete_mapping (id));

      if (mapped)
      {
        mgmt_layer_.canceled (*mapped);
      }
      else if (id == _job_id)
      {
        std::cout << "canceled [" << id << "]" << std::endl;
        set_job_status (sdpa::status::CANCELED);
      }
      else
      {
        throw std::invalid_argument ("canceled(" + id + "): no such id");
      }
    }

  private:
    void set_job_status (sdpa::status::code const c)
    {
      boost::unique_lock<boost::mutex> const _ (_mutex_job_status);
      _job_status = c;
      _condition_job_status_changed.notify_all();
    }

    void maybe_cancel_after_ms (boost::optional<std::size_t> timeout)
    {
      if (timeout)
      {
        boost::this_thread::sleep (boost::posix_time::milliseconds (*timeout));
        mgmt_layer_.cancel (_job_id, "user requested cancellation");
      }
    }

    mutable boost::mutex _mutex_job_status;
    mutable boost::condition_variable _condition_job_status_changed;
    boost::recursive_mutex _mutex_id;
    unsigned long _id;
    we::mgmt::layer mgmt_layer_;
    mutable boost::recursive_mutex _mutex_id_map;
    id_map_t  id_map_;
    fhg::thread::queue<job_t> jobs_;
    std::vector<boost::thread*> worker_;
    we::loader::loader* _loader;
    sdpa::status::code _job_status;
    boost::optional<std::string> _result;
    we::mgmt::layer::id_type _job_id;
    boost::thread _timeout_thread;
  };
}

int main (int argc, char **argv)
try
{
  namespace po = boost::program_options;

  FHGLOG_SETUP();

  po::options_description desc ("options");

  std::string path_to_act;
  std::vector<std::string> mod_path;
  std::vector<std::string> mods_to_load;
  std::size_t num_worker (8);
  std::string output;
  bool show_dots (false);
  boost::optional<std::size_t> cancel_after;

  desc.add_options()
    ("help,h", "this message")
    ("version,V", "print version information")
    ( "net"
    , po::value<std::string>(&path_to_act)->default_value("-")
    , "path to encoded activity or - for stdin"
    )
    ( "mod-path,L"
    , po::value<std::vector<std::string> >(&mod_path)
    , "where can modules be located"
    )
    ( "worker"
    , po::value<std::size_t>(&num_worker)->default_value(num_worker)
    , "number of workers"
    )
    ( "load"
    , po::value<std::vector<std::string> >(&mods_to_load)
    , "modules to load a priori"
    )
    ( "output,o"
    , po::value<std::string>(&output)->default_value(output)
    , "where to write the result pnet to"
    )
    ( "show-dots,d"
    , po::value<bool>(&show_dots)->default_value(show_dots)
    , "show dots while waiting for progress"
    )
    ( "cancel-after"
    , po::value<std::size_t>()
    , "cancel the workflow after this many milliseconds"
    )
    ;

  po::positional_options_description p;
  p.add ("input", -1);

  po::variables_map vm;
  po::store ( po::command_line_parser(argc, argv)
            . options(desc).positional(p).run()
            , vm
            );
  po::notify (vm);

  if (vm.count ("help"))
  {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count ("version"))
  {
    std::cout << fhg::project_info ("Parallel Workflow Execution");

    return EXIT_SUCCESS;
  }

  if (vm.count ("cancel-after"))
  {
    cancel_after = vm ["cancel-after"].as<std::size_t>();
  }

  we::loader::loader loader;

  BOOST_FOREACH (std::string const& m, mods_to_load)
  {
    loader.load (m, m);
  }

  BOOST_FOREACH (std::string const& p, mod_path)
  {
    loader.append_search_path (p);
  }

  sdpa_daemon const daemon
    ( num_worker
    , &loader
    , path_to_act == "-"
    ? we::mgmt::type::activity_t (std::cin)
    : we::mgmt::type::activity_t (boost::filesystem::path (path_to_act))
    , cancel_after
    );

  sdpa::status::code const rc (daemon.wait_while_job_is_running());

  switch (rc)
  {
  case sdpa::status::FINISHED:
  case sdpa::status::FAILED:
    if (output.size())
    {
      if (output == "-")
      {
        std::cout << daemon.result();
      }
      else
      {
        boost::filesystem::ofstream ofs (output);
        ofs << daemon.result();
      }
    }
    break;
  case sdpa::status::CANCELED:
    break;
  default:
    throw std::runtime_error ("STRANGE: is_running is not consistent!?");
  }

  return rc;
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  return EX_SOFTWARE;
}
