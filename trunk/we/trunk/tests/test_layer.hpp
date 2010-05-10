/*
 * =====================================================================================
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
 * =====================================================================================
 */

#ifndef WE_TESTS_TEST_LAYER_HPP
#define WE_TESTS_TEST_LAYER_HPP 1

#include <sstream>
#include <boost/bind.hpp>
#include <we/util/show.hpp>
#include <we/mgmt/layer.hpp>

#include <we/we.hpp>
#include <kdm/kdm_simple.hpp>
#include <kdm/module.hpp>

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
        return ::util::show ( id );
      }
    private:
      id_generator<unsigned long> number;
    };

    template <typename Daemon, typename IdType>
    struct context
    {
      typedef IdType id_type;
      typedef we::transition_t::net_type net_t;
      typedef we::transition_t::mod_type mod_t;
      typedef we::transition_t::expr_type expr_t;

      void handle_internally ( we::activity_t &act, net_t &n)
      {
        handle_externally (act, n);
      }

      void handle_internally ( we::activity_t &, const mod_t &)
      {
        throw std::runtime_error ( "NO internal mod here!" );
      }

      void handle_internally ( we::activity_t &, const expr_t &)
      {
        throw std::runtime_error ( "NO internal expr here!" );
      }

      void handle_externally (we::activity_t &act, net_t &)
      {
        id_type new_id ( daemon.gen_id() );
        daemon.add_mapping ( id, new_id );
        daemon.layer().submit (new_id,  we::util::text_codec::encode(act));
      }

      void handle_externally (we::activity_t & act, const mod_t &mod)
      {
        module::call ( act, mod );
        daemon.layer().finished (id, we::util::text_codec::encode(act));
      }

      void handle_externally (we::activity_t &, const expr_t &)
      {
        throw std::runtime_error ( "NO external expr here!" );
      }

      context ( Daemon & d, const id_type & an_id)
        : daemon (d)
        , id(an_id)
      {}

    private:
      Daemon & daemon;
      id_type id;
    };
  }

  template <typename Layer>
  struct sdpa_daemon
  {
    typedef Layer layer_type;
    typedef sdpa_daemon<Layer> this_type;
    typedef typename layer_type::id_type id_type;
    typedef std::map<id_type, id_type> id_map_t;

    sdpa_daemon()
      : mgmt_layer_(this, boost::bind(&sdpa_daemon::gen_id, this))
    {}

    id_type gen_id() { return ++id_; }
    void add_mapping ( const id_type & old_id, const id_type & new_id)
    {
      id_map_[new_id] = old_id;
    }

    void submit(const id_type & id, const std::string & desc)
    {
      std::cout << "submit[" << id << "]" << std::endl;
      we::activity_t act ( we::util::text_codec::decode<we::activity_t> (desc) );
      detail::context<this_type, id_type> ctxt (*this, id);
      act.execute (ctxt);
    }
    bool cancel(const id_type & id, const std::string & desc)
    {
      std::cout << "cancel[" << id << "] = " << desc << std::endl; return true;
      return true;
    }
    bool finished(const id_type & id, const std::string & desc)
    {
      if (id_map_.find (id) != id_map_.end())
      {
        // inform layer
        mgmt_layer_.finished ( id_map_[id], desc );
        id_map_.erase (id);
      }
      else
      {
        mgmt_layer_.print_statistics (std::cout);
        we::activity_t act ( we::util::text_codec::decode<we::activity_t> (desc) );
        we::mgmt::type::detail::printer <we::activity_t> p (act, std::cout);
        p << "finished [" << id << "] = " << act.output() << std::endl;
      }
      return true;
    }
    bool failed(const id_type & id, const std::string & desc)
    {
      if (id_map_.find (id) != id_map_.end())
      {
        // inform layer
        mgmt_layer_.failed ( id_map_[id], desc );
        id_map_.erase (id);
      }
      else
      {
        std::cout << "failed[" << id << "] = " << std::endl; return true;
      }
      return true;
    }
    bool cancelled(const id_type & id)
    {
      if (id_map_.find (id) != id_map_.end())
      {
        // inform layer
        mgmt_layer_.cancelled ( id_map_[id] );
        id_map_.erase (id);
      }
      else
      {
        std::cout << "cancelled[" << id << "]" << std::endl; return true;
      }
      return true;
    }

    inline layer_type & layer() { return mgmt_layer_; }

  private:
    detail::id_generator<id_type> id_;
    layer_type mgmt_layer_;
    id_map_t  id_map_;
  };
}

#endif
