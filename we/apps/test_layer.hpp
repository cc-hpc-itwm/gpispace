/*
 * =============================================================================
 *
 *       Filename:  test_layer.hpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  03/02/2010 02:53:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =============================================================================
 */

#ifndef WE_TESTS_TEST_LAYER_HPP
#define WE_TESTS_TEST_LAYER_HPP 1

#include <sstream>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_map.hpp>
#include <fhg/util/show.hpp>
#include <fhg/error_codes.hpp>
#include <we/mgmt/layer.hpp>
#include <we/mgmt/bits/queue.hpp>

#include <we/loader/loader.hpp>
#include <we/mgmt/context.hpp>

#include <we/loader/module_call.hpp>
#include <we/type/requirement.hpp>

#include <we/type/module_call.fwd.hpp>
#include <we/type/expression.fwd.hpp>
#include <we/type/net.fwd.hpp>

#include <list>

namespace test {
  namespace detail
  {
    template <typename I>
    struct id_generator
    {
      explicit
      id_generator(I initial=0) : id(initial) {}

      inline I operator++() { return ++id; }
    private:
      I id;
    };

    template <>
    struct id_generator<std::string>
    {
      inline const std::string operator++()
      {
        unsigned long id = ++number;
        return fhg::util::show ( id );
      }
    private:
      id_generator<unsigned long> number;
    };

    template <typename Daemon, typename IdType>
    struct context : public we::mgmt::context
    {
      typedef IdType id_type;

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
        id_type new_id (daemon.gen_id());
        daemon.add_mapping (id, new_id);
        daemon.layer().submit (new_id,  act.to_string());
        return 0;
      }

      virtual int handle_externally (we::mgmt::type::activity_t& act, mod_t& mod)
      {
        try
        {
          module::call (daemon.loader(), act, mod);
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

      context (Daemon & d, const id_type & an_id)
        : daemon (d)
        , id(an_id)
     {}

    private:
      Daemon& daemon;
      id_type id;
    };
  }

  template <typename IdType>
  struct job_t
  {
    job_t ()
    {}

    job_t (const job_t<IdType> & other)
      : id (other.id)
      , desc(other.desc)
    { }

    job_t (const IdType & id_, const std::string & desc_)
      : id (id_)
      , desc(desc_)
    { }

    IdType id;
    std::string desc;
  };

  template <typename Layer>
  struct sdpa_daemon
  {
    typedef Layer layer_type;
    typedef sdpa_daemon<Layer> this_type;
    typedef typename layer_type::id_type id_type;
    typedef boost::unordered_map<id_type, id_type> id_map_t;
    typedef job_t<id_type> job_type;
    typedef we::mgmt::detail::queue<job_type, 8> job_q_t;
    typedef std::vector<boost::thread*> worker_list_t;

    explicit
    sdpa_daemon(std::size_t num_worker = 1)
      : mgmt_layer_(this, boost::bind(&sdpa_daemon::gen_id, this))
      , next_worker_(0)
    {
      start(num_worker);
    }

    ~sdpa_daemon()
    {
      stop();
    }

    void start(const std::size_t num_worker = 1)
    {
      jobs_ = new job_q_t[num_worker];
      for (std::size_t n (0); n < num_worker; ++n)
      {
        worker_.push_back( new boost::thread( boost::bind(&this_type::worker, this, n)));
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
      delete [] jobs_;
      jobs_ = NULL;
    }

    void worker(const std::size_t rank)
    {
      std::cout << "SDPA Layer worker-" << rank << " started." << std::endl;
      for (;;)
      {
        job_type job (jobs_[rank].get());

        we::mgmt::type::activity_t act ( we::util::codec::decode (job.desc));
        detail::context<this_type, id_type> ctxt (*this, job.id);
        act.execute (&ctxt);
      }
      std::cout << "SDPA Layer worker-" << rank << " stopped." << std::endl;
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
               )
    {
      job_type job (id, desc);
      jobs_[next_worker_] .put(job);
      next_worker_ = (next_worker_ + 1) % worker_.size();
    }

    bool cancel(const id_type & id, const std::string & desc)
    {
      std::cout << "cancel[" << id << "] = " << desc << std::endl;

      try
      {
        id_type mapped_id (get_mapping (id));
        del_mapping (id);

        // inform layer
        mgmt_layer_.cancelled (mapped_id);
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
        we::mgmt::type::activity_t act (we::util::codec::decode (desc));

        std::cout << "finished [" << id << "] = ";
        act.print (std::cout, act.output());
        std::cout << std::endl;
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
        we::mgmt::type::activity_t act (we::util::codec::decode (desc));

        std::cout << "failed [" << id << "] = ";
        act.print (std::cout, act.output());
        std::cout << " error-code := " << error_code
                  << " reason := " << reason
                  << std::endl;
      }
      return true;
    }

    bool cancelled(const id_type & id)
    {
      try
      {
        id_type mapped_id (get_mapping (id));
        del_mapping (id);

        // inform layer
        mgmt_layer_.cancelled (mapped_id);
      }
      catch (std::out_of_range const &)
      {
        std::cout << "cancelled [" << id << "]" << std::endl;
      }
      return true;
    }

    inline layer_type & layer() { return mgmt_layer_; }
    inline we::loader::loader & loader() { return loader_; }

  private:
    boost::recursive_mutex mutex_;
    detail::id_generator<id_type> id_;
    layer_type mgmt_layer_;
    std::size_t next_worker_;
    id_map_t  id_map_;
    job_q_t *jobs_;
    worker_list_t worker_;
    we::loader::loader loader_;
  };
}

#endif
