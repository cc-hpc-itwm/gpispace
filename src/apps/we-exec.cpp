
#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/mgmt/bits/queue.hpp>
#include <we/mgmt/context.hpp>
#include <we/mgmt/layer.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/module_call.fwd.hpp>
#include <we/type/net.fwd.hpp>
#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>
#include <we/type/value/show.hpp>

#include <fhg/error_codes.hpp>
#include <fhg/revision.hpp>
#include <fhg/util/getenv.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/stat.hpp>
#include <fhglog/fhglog.hpp>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <stdint.h>

namespace
{
  struct sdpa_daemon;

  struct context : public we::mgmt::context
  {
    virtual int handle_internally (we::mgmt::type::activity_t&, net_t&);
    virtual int handle_internally (we::mgmt::type::activity_t&, mod_t&);
    virtual int handle_internally (we::mgmt::type::activity_t&, expr_t&);
    virtual int handle_externally (we::mgmt::type::activity_t&, net_t&);
    virtual int handle_externally (we::mgmt::type::activity_t&, mod_t&);
    virtual int handle_externally (we::mgmt::type::activity_t&, expr_t&);

    context (sdpa_daemon& d, const we::mgmt::layer::id_type& an_id)
      : daemon (d)
      , id (an_id)
    {}

  private:
    sdpa_daemon& daemon;
    we::mgmt::layer::id_type id;
  };

  struct job_t
  {
    job_t ()
    {}

    job_t (const we::mgmt::layer::id_type& id_, const std::string & desc_)
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
    typedef we::mgmt::detail::queue<job_t, 8> job_q_t;
    typedef std::vector<boost::thread*> worker_list_t;

    explicit
    sdpa_daemon (std::size_t num_worker)
      : mgmt_layer_ (this, boost::bind (&sdpa_daemon::gen_id, this))
    {
      start (num_worker);
    }

    ~sdpa_daemon()
    {
      stop();
    }

    void start (const std::size_t num_worker = 1)
    {
      for (std::size_t n (0); n < num_worker; ++n)
      {
        worker_.push_back
          (new boost::thread (boost::bind (&sdpa_daemon::worker, this, n)));
      }
    }

    void stop()
    {
      BOOST_FOREACH (boost::thread* t, worker_)
      {
        t->interrupt();
        t->join();
        delete t;
      }
      worker_.clear();
    }

    void worker (const std::size_t rank)
    {
      MLOG (INFO, "SDPA layer worker-" << rank << " started");

      for (;;)
      {
        MLOG (TRACE, "worker-" << rank << " idle");

        job_t const job (jobs_.get());

        we::mgmt::type::activity_t act (job.desc);

        MLOG ( TRACE
             , "worker-" << rank << " busy with " << act.transition().name()
             );

        context ctxt (*this, job.id);
        act.execute (&ctxt);
      }

      MLOG (INFO, "SDPA layer worker-" << rank << " stopped");
    }

    we::mgmt::layer::id_type gen_id()
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex_id);

      return boost::lexical_cast<std::string> (++_id);
    }
    void add_mapping ( const we::mgmt::layer::id_type& old_id
                     , const we::mgmt::layer::id_type& new_id
                     )
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex_id_map);

      id_map_[new_id] = old_id;
    }
    we::mgmt::layer::id_type get_mapping (const we::mgmt::layer::id_type& id)
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex_id_map);

      return id_map_.at (id);
    }
    void del_mapping (const we::mgmt::layer::id_type& id)
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex_id_map);

      id_map_.erase (id);
    }

    void submit ( const we::mgmt::layer::id_type& id
                , const std::string & desc
                , std::list<we::type::requirement_t> const&
                , const we::type::schedule_data&
                , const we::type::user_data&
                )
    {
      jobs_.put (job_t (id, desc));
    }

    bool cancel (const we::mgmt::layer::id_type& id, const std::string & desc)
    {
      std::cout << "cancel[" << id << "] = " << desc << std::endl;

      try
      {
        we::mgmt::layer::id_type mapped_id (get_mapping (id));
        del_mapping (id);

        mgmt_layer_.canceled (mapped_id);
        return true;
      }
      catch (std::exception const &ex)
      {
        return false;
      }
    }

    bool finished (const we::mgmt::layer::id_type& id, const std::string & desc)
    {
      try
      {
        we::mgmt::layer::id_type mapped_id (get_mapping (id));
        del_mapping (id);

        mgmt_layer_.finished (mapped_id, desc);
      }
      catch (std::out_of_range const &)
      {
        we::mgmt::type::activity_t act (desc);

        std::cout << "finished [" << id << "]" << std::endl;
        BOOST_FOREACH ( const we::mgmt::type::activity_t::token_on_port_t& top
                      , act.output()
                      )
        {
          std::cout << act.transition().get_port (top.second).name()
                    << " => " << pnet::type::value::show (top.first) << std::endl;
        }
      }
      return true;
    }

    bool failed ( const we::mgmt::layer::id_type& id
                , const std::string & desc
                , const int error_code
                , const std::string & reason
                )
    {
      try
      {
        we::mgmt::layer::id_type mapped_id (get_mapping (id));
        del_mapping (id);

        mgmt_layer_.failed (mapped_id, desc, error_code, reason);
      }
      catch (std::out_of_range const &)
      {
        we::mgmt::type::activity_t act (desc);

        std::cout << "failed [" << id << "] = ";
        act.print (std::cout, act.output());
        std::cout << " error-code := " << error_code
                  << " reason := " << reason
                  << std::endl;
      }
      return true;
    }

    bool canceled (const we::mgmt::layer::id_type& id)
    {
      try
      {
        we::mgmt::layer::id_type mapped_id (get_mapping (id));
        del_mapping (id);

        mgmt_layer_.canceled (mapped_id);
      }
      catch (std::out_of_range const &)
      {
        std::cout << "canceled [" << id << "]" << std::endl;
      }
      return true;
    }

    inline we::mgmt::layer& layer()
    {
      return mgmt_layer_;
    }
    inline we::loader::loader& loader()
    {
      return loader_;
    }

  private:
    boost::recursive_mutex _mutex_id;
    unsigned long _id;
    we::mgmt::layer mgmt_layer_;
    boost::recursive_mutex _mutex_id_map;
    id_map_t  id_map_;
    job_q_t jobs_;
    worker_list_t worker_;
    we::loader::loader loader_;
  };

  int context::handle_internally (we::mgmt::type::activity_t& act, net_t& n)
  {
    return handle_externally (act, n);
  }
  int context::handle_internally (we::mgmt::type::activity_t&, mod_t&)
  {
    throw std::runtime_error ("NO internal mod here!");
  }
  int context::handle_internally (we::mgmt::type::activity_t&, expr_t&)
  {
    throw std::runtime_error ("NO internal expr here!");
  }
  int context::handle_externally (we::mgmt::type::activity_t& act, net_t&)
  {
    we::mgmt::layer::id_type const new_id (daemon.gen_id());
    daemon.add_mapping (id, new_id);
    daemon.layer().submit (new_id,  act, we::type::user_data());
    return 0;
  }
  int context::handle_externally (we::mgmt::type::activity_t& act, mod_t& mod)
  {
    try
    {
      //!\todo pass a real gspc::drts::context
      module::call (daemon.loader(), 0, act, mod);
      daemon.layer().finished (id, act.to_string());
    }
    catch (std::exception const & ex)
    {
      daemon.layer().failed ( id
                            , act.to_string()
                            , fhg::error::MODULE_CALL_FAILED
                            , ex.what()
                            );
    }
    return 0;
  }
  int context::handle_externally (we::mgmt::type::activity_t&, expr_t&)
  {
    throw std::runtime_error ("NO external expr here!");
  }
}

typedef std::string id_type;

typedef we::mgmt::layer::internal_id_type layer_id_type;

static std::vector<id_type> jobs;
static std::set<layer_id_type> layer_jobs;
static boost::recursive_mutex mutex;
static std::string encoded_result;

namespace observe
{
  void submitted (const we::mgmt::layer *l, layer_id_type const & id)
  {
    boost::unique_lock<boost::recursive_mutex> const _ (mutex);

    std::cerr << "submitted: " << id << std::endl;
    layer_jobs.insert (id);
  }

  void finished ( const we::mgmt::layer *l
                , layer_id_type const& id
                , std::string const& s
                )
  {
    boost::unique_lock<boost::recursive_mutex> const _ (mutex);

    if (layer_jobs.find (id) != layer_jobs.end())
    {
      layer_jobs.erase (id);
      we::mgmt::type::activity_t act (s);
      std::cerr << "job finished: " << act.transition().name() << "-" << id << std::endl;
      encoded_result = s;
    }
  }
  void failed ( const we::mgmt::layer *l
              , layer_id_type const& id
              , std::string const& s
              )
  {
    boost::unique_lock<boost::recursive_mutex> const _ (mutex);

    if (layer_jobs.find (id) != layer_jobs.end())
    {
      layer_jobs.erase (id);
      we::mgmt::type::activity_t act (s);
      std::cerr << "job failed: " << act.transition().name() << "-" << id << std::endl;
      encoded_result = s;
    }
  }
  void canceled ( const we::mgmt::layer *l
                , layer_id_type const& id
                , std::string const& s
                )
  {
    boost::unique_lock<boost::recursive_mutex> const _ (mutex);

    if (layer_jobs.find (id) != layer_jobs.end())
    {
      layer_jobs.erase (id);
      we::mgmt::type::activity_t act (s);
      std::cerr << "job canceled: " << act.transition().name() << "-" << id << std::endl;
      encoded_result = s;
    }
  }
}

int main (int argc, char **argv)
try
{
  namespace po = boost::program_options;

  fhg::log::Configurator::configure();

  po::options_description desc ("options");

  std::string path_to_act;
  std::string mod_path;
  std::vector<std::string> mods_to_load;
  std::size_t num_worker (8);
  std::string output;
  bool show_dots (false);

  desc.add_options()
    ("help,h", "this message")
    ("version,V", "print version information")
    ( "net"
    , po::value<std::string>(&path_to_act)->default_value("-")
    , "path to encoded activity or - for stdin"
    )
    ( "mod-path,L"
    , po::value<std::string>(&mod_path)->default_value
        (fhg::util::getenv("PC_LIBRARY_PATH", "."))
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

  sdpa_daemon daemon (num_worker);

  BOOST_FOREACH (std::string const& m, mods_to_load)
  {
    daemon.loader().load (m);
  }

  {
    std::vector<std::string> search_path;
    fhg::log::split (mod_path, ":", std::back_inserter (search_path));
    BOOST_FOREACH (std::string const &p, search_path)
    {
      daemon.loader().append_search_path (p);
    }
  }
  we::mgmt::layer& mgmt_layer (daemon.layer());

  mgmt_layer.sig_submitted.connect (&observe::submitted);
  mgmt_layer.sig_finished.connect (&observe::finished);
  mgmt_layer.sig_failed.connect (&observe::failed);
  mgmt_layer.sig_canceled.connect (&observe::canceled);

  we::mgmt::type::activity_t act
    ( path_to_act == "-"
    ? we::mgmt::type::activity_t (std::cin)
    : we::mgmt::type::activity_t (boost::filesystem::path (path_to_act))
    );

  we::mgmt::layer::id_type id (daemon.gen_id());
  jobs.push_back (id);
  mgmt_layer.submit (id, act, we::type::user_data());

  while (layer_jobs.size() > 0)
  {
    if (show_dots)
    {
      std::cerr << "." << std::flush;
    }
    sleep (1);
  }

  std::cerr << "Everything done." << std::endl;

  FHG_UTIL_STAT_OUT (std::cerr);

  if (output.size ())
  {
    if (output == "=")
    {
      std::cout << encoded_result;
    }
    else
    {
      std::ofstream ofs (output.c_str ());
      ofs << encoded_result;
    }
  }

  return ((jobs.size() == 0) ? EXIT_SUCCESS : EXIT_FAILURE);
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
