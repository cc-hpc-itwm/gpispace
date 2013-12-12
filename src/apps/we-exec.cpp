
#include <we/loader/loader.hpp>
#include <we/loader/module_call.hpp>
#include <we/mgmt/context.hpp>
#include <we/mgmt/layer.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/module_call.fwd.hpp>
#include <we/type/net.fwd.hpp>
#include <we/type/requirement.hpp>
#include <we/type/schedule_data.hpp>
#include <we/type/value/show.hpp>

#include <fhg/revision.hpp>
#include <fhg/util/getenv.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/stat.hpp>
#include <fhg/util/thread/queue.hpp>
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
#include <set>
#include <vector>

namespace observe
{
  class state_type
  {
  public:
    state_type ()
      : _mutex_jobs ()
      , _jobs (0)
      , _result()
    {}

    void insert (we::mgmt::detail::descriptor)
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex_jobs);

      ++_jobs;
    }
    bool done() const
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex_jobs);

      return !_jobs;
    }
    void remove (we::mgmt::detail::descriptor d)
    {
      boost::unique_lock<boost::recursive_mutex> const _ (_mutex_jobs);

      --_jobs;
      _result = d.activity().to_string();
    }
    std::string const& result() const
    {
      if (not _result)
      {
        throw std::runtime_error ("missing result");
      }

      return *_result;
    }

  private:
    mutable boost::recursive_mutex _mutex_jobs;
    unsigned long _jobs;
    boost::optional<std::string> _result;
  };
}

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

  private:
    we::mgmt::layer::id_type _id;
    we::loader::loader* _loader;
    we::mgmt::layer* _layer;
    boost::function<we::mgmt::layer::id_type (we::mgmt::layer::id_type const&)>
      _new_id;
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
    typedef fhg::thread::queue<job_t> job_q_t;
    typedef std::vector<boost::thread*> worker_list_t;

    sdpa_daemon ( std::size_t num_worker
                , we::loader::loader* loader
                , observe::state_type* observer
                , we::mgmt::type::activity_t const act
                )
        : _mutex_id()
        , _id (0)
        , mgmt_layer_ (this, boost::bind (&sdpa_daemon::gen_id, this))
        , _mutex_id_map()
        , id_map_()
        , jobs_()
        , worker_()
        , _loader (loader)
    {
      mgmt_layer_.sig_insert =
        boost::bind (&observe::state_type::insert, observer, _1);
      mgmt_layer_.sig_remove =
        boost::bind (&observe::state_type::remove, observer, _1);

      for (std::size_t n (0); n < num_worker; ++n)
      {
        worker_.push_back
          (new boost::thread (boost::bind (&sdpa_daemon::worker, this, n)));
      }

      mgmt_layer_.submit (gen_id(), act, we::type::user_data());
    }

    ~sdpa_daemon()
    {
      BOOST_FOREACH (boost::thread* t, worker_)
      {
        t->interrupt();
        t->join();
        delete t;
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
    we::mgmt::layer::id_type
      get_mapping (const we::mgmt::layer::id_type& id) const
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
        we::mgmt::layer::id_type const mapped_id (get_mapping (id));
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
        we::mgmt::layer::id_type const mapped_id (get_mapping (id));
        del_mapping (id);

        mgmt_layer_.finished (mapped_id, desc);
      }
      catch (std::out_of_range const &)
      {
        we::mgmt::type::activity_t const act (desc);

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
        we::mgmt::layer::id_type const mapped_id (get_mapping (id));
        del_mapping (id);

        mgmt_layer_.failed (mapped_id, desc, error_code, reason);
      }
      catch (std::out_of_range const &)
      {
        we::mgmt::type::activity_t const act (desc);

        std::cout << "failed [" << id << "] = ";
        act.print (std::cout, act.output());
        std::cout << " error-code := " << error_code
                  << " reason := " << reason
                  << " activity := " << act.transition ().name ()
                  << std::endl;
      }
      return true;
    }

    bool canceled (const we::mgmt::layer::id_type& id)
    {
      try
      {
        we::mgmt::layer::id_type const mapped_id (get_mapping (id));
        del_mapping (id);

        mgmt_layer_.canceled (mapped_id);
      }
      catch (std::out_of_range const &)
      {
        std::cout << "canceled [" << id << "]" << std::endl;
      }
      return true;
    }

  private:
    boost::recursive_mutex _mutex_id;
    unsigned long _id;
    we::mgmt::layer mgmt_layer_;
    mutable boost::recursive_mutex _mutex_id_map;
    id_map_t  id_map_;
    job_q_t jobs_;
    worker_list_t worker_;
    we::loader::loader* _loader;
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
    _layer->submit (_new_id (_id),  act, we::type::user_data());
    return 0;
  }
  int context::handle_externally (we::mgmt::type::activity_t& act, mod_t& mod)
  {
    try
    {
      //!\todo pass a real gspc::drts::context
      module::call (*_loader, 0, act, mod);
      _layer->finished (_id, act.to_string());
    }
    catch (std::exception const & ex)
    {
      std::cerr << "handle-ext(" << act.transition ().name () << ") failed: " << ex.what () << std::endl;

      _layer->failed ( _id
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

  we::loader::loader loader;

  BOOST_FOREACH (std::string const& m, mods_to_load)
  {
    loader.load (m);
  }

  {
    std::vector<std::string> search_path;
    fhg::log::split (mod_path, ":", std::back_inserter (search_path));
    BOOST_FOREACH (std::string const &p, search_path)
    {
      loader.append_search_path (p);
    }
  }

  observe::state_type observer;

  sdpa_daemon const daemon
    ( num_worker
    , &loader
    , &observer
    , path_to_act == "-"
    ? we::mgmt::type::activity_t (std::cin)
    : we::mgmt::type::activity_t (boost::filesystem::path (path_to_act))
    );

  while (not observer.done())
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
      std::cout << observer.result();
    }
    else
    {
      std::ofstream ofs (output.c_str ());
      ofs << observer.result();
    }
  }

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
