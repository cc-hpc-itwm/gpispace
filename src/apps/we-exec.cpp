
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

namespace test {
  namespace detail
  {
    struct id_generator
    {
      id_generator() : _n (0) {}

      inline const std::string operator++()
      {
        return boost::lexical_cast<std::string> (++_n);
      }

    private:
      unsigned long _n;
    };

    template <typename Daemon>
    struct context : public we::mgmt::context
    {
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
        we::mgmt::layer::id_type new_id (daemon.gen_id());
        daemon.add_mapping (id, new_id);
        we::type::user_data ud;
        daemon.layer().submit (new_id,  act, ud);
        return 0;
      }

      virtual int handle_externally (we::mgmt::type::activity_t& act, mod_t& mod)
      {
        try
        {
          //!\todo pass a real gspc::drts::context
          module::call (daemon.loader(), 0, act, mod);
          daemon.layer().finished (id, act.to_string());
        }
        catch (std::exception const & ex)
        {
          daemon.layer().failed (id
                                , act.to_string()
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

      context (Daemon & d, const we::mgmt::layer::id_type& an_id)
        : daemon (d)
        , id(an_id)
     {}

    private:
      Daemon& daemon;
      we::mgmt::layer::id_type id;
    };
  }

  struct job_t
  {
    job_t ()
    {}

    job_t (const we::mgmt::layer::id_type& id_, const std::string & desc_)
      : id (id_)
      , desc(desc_)
    { }

    we::mgmt::layer::id_type id;
    std::string desc;
  };

  struct sdpa_daemon
  {
    typedef we::mgmt::layer::id_type id_type;
    typedef boost::unordered_map<id_type, id_type> id_map_t;
    typedef we::mgmt::detail::queue<job_t, 8> job_q_t;
    typedef std::vector<boost::thread*> worker_list_t;

    explicit
    sdpa_daemon (std::size_t num_worker)
      : mgmt_layer_(this, boost::bind(&sdpa_daemon::gen_id, this))
    {
      start(num_worker);
    }

    ~sdpa_daemon()
    {
      stop();
    }

    void start (const std::size_t num_worker = 1)
    {
      for (std::size_t n (0); n < num_worker; ++n)
      {
        worker_.push_back( new boost::thread( boost::bind(&sdpa_daemon::worker, this, n)));
      }
    }

    void stop()
    {
      for (std::vector<boost::thread*>::iterator it (worker_.begin()); it != worker_.end(); ++it)
      {
        (*it)->interrupt();
        (*it)->join();
        delete (*it);
      }
      worker_.clear();
    }

    void worker(const std::size_t rank)
    {
      MLOG (INFO, "SDPA layer worker-" << rank << " started");

      for (;;)
      {
        MLOG (TRACE, "worker-" << rank << " idle");

        job_t job (jobs_.get());

        we::mgmt::type::activity_t act (job.desc);

        MLOG ( TRACE
             , "worker-" << rank << " busy with " << act.transition ().name ()
             );

        detail::context<sdpa_daemon> ctxt (*this, job.id);
        act.execute (&ctxt);
      }

      MLOG (INFO, "SDPA layer worker-" << rank << " stopped");
    }

    id_type gen_id()
    {
      boost::unique_lock<boost::recursive_mutex> lock (mutex_);
      return ++id_;
    }
    void add_mapping ( const id_type & old_id, const id_type & new_id)
    {
      boost::unique_lock<boost::recursive_mutex> lock (mutex_);
      id_map_[new_id] = old_id;
    }
    id_type get_mapping (const id_type & id)
    {
      boost::unique_lock<boost::recursive_mutex> lock (mutex_);
      return id_map_.at (id);
    }
    void del_mapping (const id_type & id)
    {
      boost::unique_lock<boost::recursive_mutex> lock (mutex_);
      id_map_.erase (id);
    }

    typedef std::list<we::type::requirement_t> requirement_list_t;
    void submit( const id_type & id
               , const std::string & desc
               , requirement_list_t req_list = requirement_list_t()
               , const we::type::schedule_data& = we::type::schedule_data()
               , const we::type::user_data& = we::type::user_data ()
               )
    {
      job_t job (id, desc);

      jobs_.put (job);
    }

    bool cancel(const id_type & id, const std::string & desc)
    {
      std::cout << "cancel[" << id << "] = " << desc << std::endl;

      try
      {
        id_type mapped_id (get_mapping (id));
        del_mapping (id);

        // inform layer
        mgmt_layer_.canceled (mapped_id);
        return true;
      }
      catch (std::exception const &ex)
      {
        return false;
      }
    }

    bool finished(const id_type & id, const std::string & desc)
    {
      try
      {
        id_type mapped_id (get_mapping (id));
        del_mapping (id);

        // inform layer
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

    bool failed( const id_type & id
               , const std::string & desc
               , const int error_code
               , const std::string & reason
               )
    {
      try
      {
        id_type mapped_id (get_mapping (id));
        del_mapping (id);

        // inform layer
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

    bool canceled(const id_type & id)
    {
      try
      {
        id_type mapped_id (get_mapping (id));
        del_mapping (id);

        // inform layer
        mgmt_layer_.canceled (mapped_id);
      }
      catch (std::out_of_range const &)
      {
        std::cout << "canceled [" << id << "]" << std::endl;
      }
      return true;
    }

    inline we::mgmt::layer & layer() { return mgmt_layer_; }
    inline we::loader::loader & loader() { return loader_; }

  private:
    boost::recursive_mutex mutex_;
    detail::id_generator id_;
    we::mgmt::layer mgmt_layer_;
    id_map_t  id_map_;
    job_q_t jobs_;
    worker_list_t worker_;
    we::loader::loader loader_;
  };
}

using namespace test;
namespace po = boost::program_options;

typedef std::string id_type;

typedef sdpa_daemon daemon_type;
typedef we::mgmt::layer::internal_id_type layer_id_type;

static std::vector<id_type> jobs;
static std::set<layer_id_type> layer_jobs;
static boost::recursive_mutex mutex;
typedef boost::unique_lock<boost::recursive_mutex> lock_t;
static bool verbose (false);
static std::string encoded_result;

// observe workflow engine
static
  void observe_submitted (const we::mgmt::layer *l, layer_id_type const & id)
{
  {
    lock_t lock(mutex);

    std::cerr << "submitted: " << id << std::endl;
    layer_jobs.insert (id);
  }

  if (verbose)
    l->print_statistics(std::cerr);
}

static
  void observe_finished (const we::mgmt::layer *l, layer_id_type const & id, std::string const &s)
{
  {
    lock_t lock(mutex);

    if (layer_jobs.find (id) != layer_jobs.end())
    {
      layer_jobs.erase (id);
      we::mgmt::type::activity_t act (s);
      std::cerr << "job finished: " << act.transition().name() << "-" << id << std::endl;
      encoded_result = s;
    }
  }

  if (verbose)
    l->print_statistics(std::cerr);
}
static
void observe_failed (const we::mgmt::layer *l, layer_id_type const & id, std::string const &s)
{
  {
    lock_t lock(mutex);

    if (layer_jobs.find (id) != layer_jobs.end())
    {
      layer_jobs.erase (id);
      we::mgmt::type::activity_t act (s);
      std::cerr << "job failed: " << act.transition().name() << "-" << id << std::endl;
      encoded_result = s;
    }
  }

  if (verbose)
    l->print_statistics(std::cerr);
}
static
void observe_canceled (const we::mgmt::layer *l, layer_id_type const & id, std::string const &s)
{
  {
    lock_t lock(mutex);

    if (layer_jobs.find (id) != layer_jobs.end())
    {
      layer_jobs.erase (id);
      we::mgmt::type::activity_t act (s);
      std::cerr << "job canceled: " << act.transition().name() << "-" << id << std::endl;
      encoded_result = s;
    }
  }

  if (verbose)
    l->print_statistics(std::cerr);
}

static
void observe_executing (const we::mgmt::layer *l, layer_id_type const & id)
{
  std::cerr << "activity executing: id := " << id << std::endl;
  if (verbose)
    l->print_statistics(std::cerr);
}

int main (int argc, char **argv)
try
{
  fhg::log::Configurator::configure();

  po::options_description desc("options");

  std::string path_to_act;
  std::string mod_path;
  std::vector<std::string> mods_to_load;
  std::size_t num_worker (8);
  std::string output;
  bool show_dots (false);

  desc.add_options()
    ("help,h", "this message")
    ("version,V", "print version information")
    ("verbose,v", "be verbose")
    ("net", po::value<std::string>(&path_to_act)->default_value("-"), "path to encoded activity or - for stdin")
    ( "mod-path,L"
    , po::value<std::string>(&mod_path)->default_value
        (fhg::util::getenv("PC_LIBRARY_PATH", "."))
    , "where can modules be located"
    )
    ("worker", po::value<std::size_t>(&num_worker)->default_value(num_worker), "number of workers")
    ("load", po::value<std::vector<std::string> >(&mods_to_load), "modules to load a priori")
    ("output,o", po::value<std::string>(&output)->default_value(output), "where to write the result pnet to")
    ("show-dots,d", po::value<bool>(&show_dots)->default_value(show_dots), "show dots while waiting for progress")
    ;

  po::positional_options_description p;
  p.add("input", -1);

  po::variables_map vm;
  po::store( po::command_line_parser(argc, argv)
           . options(desc).positional(p).run()
           , vm
           );
  po::notify (vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
    }

  if (vm.count("version"))
  {
    std::cout << fhg::project_info ("Parallel Workflow Execution");

    return EXIT_SUCCESS;
  }

  // instantiate daemon and layer
  daemon_type daemon(num_worker);

  for ( std::vector<std::string>::const_iterator m (mods_to_load.begin())
      ; m != mods_to_load.end()
      ; ++m
      )
  {
    daemon.loader().load (*m);
  }

  {
    std::vector<std::string> search_path;
    fhg::log::split (mod_path, ":", std::back_inserter (search_path));
    BOOST_FOREACH (std::string const &p, search_path)
    {
      daemon.loader().append_search_path (p);
    }
  }
  we::mgmt::layer& mgmt_layer = daemon.layer();

  mgmt_layer.sig_submitted.connect ( &observe_submitted );
  mgmt_layer.sig_finished.connect  ( &observe_finished );
  mgmt_layer.sig_failed.connect    ( &observe_failed );
  mgmt_layer.sig_canceled.connect ( &observe_canceled );

  if (vm.count ("verbose"))
  {
    verbose = true;
    mgmt_layer.sig_executing.connect ( &observe_executing );
  }

  we::mgmt::type::activity_t act
    ( path_to_act == "-"
    ? we::mgmt::type::activity_t (std::cin)
    : we::mgmt::type::activity_t (boost::filesystem::path (path_to_act))
    );

  daemon_type::id_type id = daemon.gen_id();
  jobs.push_back(id);
  mgmt_layer.submit(id, act, we::type::user_data ());

#if 0
  size_t max_wait (5);

  while (jobs.size() > 0 && (--max_wait > 0))
    {
      if (show_dots) { std::cerr << "." << std::flush; }
      sleep (1);
    }
#else
  while (layer_jobs.size() > 0)
    {
      if (show_dots) { std::cerr << "." << std::flush; }
      sleep (1);
    }
#endif

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
