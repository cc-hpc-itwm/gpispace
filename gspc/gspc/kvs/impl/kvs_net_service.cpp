#include "kvs_net_service.hpp"

#include <iostream>
#include <string>

#include <gspc/net.hpp>
#include <gspc/kvs/kvs.hpp>

#include <fhg/util/read.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <boost/signals2.hpp>
#include <boost/optional.hpp>

#include <gspc/net/server/default_queue_manager.hpp>

namespace gspc
{
  namespace kvs
  {
    service_t::service_t ()
      : m_kvs (gspc::kvs::create ("inproc://"))
    {
      m_on_change_connection =
        m_kvs->onChange.connect (boost::bind ( &service_t::on_change
                                             , this
                                             , _1
                                             , _2
                                             )
                                );
      setup_rpc_handler ();
    }

    service_t::service_t (std::string const &url)
      : m_kvs (gspc::kvs::create (url))
    {
      m_on_change_connection =
        m_kvs->onChange.connect (boost::bind ( &service_t::on_change
                                             , this
                                             , _1
                                             , _2
                                             )
                                );
      setup_rpc_handler ();
    }

    service_t::~service_t ()
    {
      m_on_change_connection.disconnect ();
    }

    void service_t::setup_rpc_handler ()
    {
      m_rpc_table ["put"]  = boost::bind (&service_t::rpc_put, this, _1);
      m_rpc_table ["get"]  = boost::bind (&service_t::rpc_get, this, _1);
      m_rpc_table ["get_regex"]  = boost::bind (&service_t::rpc_get_regex, this, _1);
      m_rpc_table ["del"]  = boost::bind (&service_t::rpc_del, this, _1);
      m_rpc_table ["del_regex"]  = boost::bind (&service_t::rpc_del_regex, this, _1);
      m_rpc_table ["set_ttl"]  = boost::bind (&service_t::rpc_set_ttl, this, _1);
      m_rpc_table ["set_ttl_regex"]  = boost::bind (&service_t::rpc_set_ttl_regex, this, _1);
      m_rpc_table ["push"]  = boost::bind (&service_t::rpc_push, this, _1);
      m_rpc_table ["wait"]  = boost::bind (&service_t::rpc_wait, this, _1);
      m_rpc_table ["try_pop"]  = boost::bind (&service_t::rpc_try_pop, this, _1);
      m_rpc_table ["counter_reset"]  = boost::bind (&service_t::rpc_counter_reset, this, _1);
      m_rpc_table ["counter_change"]  = boost::bind (&service_t::rpc_counter_change, this, _1);
      m_rpc_table ["help"] = boost::bind (&service_t::rpc_help, this, _1);
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_put (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      std::list<std::pair<api_t::key_type, api_t::value_type> >
        values_to_put;

      while (not pos.end ())
      {
        if (*pos == '\n')
        {
          ++pos;
        }
        else
        {
          const api_t::key_type key =
            fhg::util::parse::require::plain_string (pos, ' ');
          api_t::value_type val = pnet::type::value::read (pos);
          values_to_put.push_back (std::make_pair (key, val));
        }
      }

      return encode_error_code (rqst, m_kvs->put (values_to_put));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_get (gspc::net::frame const &rqst)
    {
      const api_t::key_type &key = rqst.get_body ();

      api_t::value_type val;
      int rc = m_kvs->get (key, val);

      gspc::net::frame rply = encode_error_code (rqst, rc);
      gspc::net::stream (rply)
        << pnet::type::value::show (val)
        << std::endl
        ;
      return rply;
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_get_regex (gspc::net::frame const &rqst)
    {
      std::string const &regex = rqst.get_body ();

      typedef std::list<std::pair<api_t::key_type, api_t::value_type> > kv_list_t;
      kv_list_t values;
      int rc = m_kvs->get_regex (regex, values);

      gspc::net::frame rply = encode_error_code (rqst, rc);

      if (0 == rc)
      {
        kv_list_t::const_iterator it = values.begin ();
        const kv_list_t::const_iterator end = values.end ();

        for ( ; it != end ; ++it)
        {
          gspc::net::stream (rply)
            << it->first
            << " "
            << pnet::type::value::show (it->second)
            << std::endl
            ;
        }
      }

      return rply;
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_del (gspc::net::frame const &rqst)
    {
      return
        encode_error_code (rqst, m_kvs->del (rqst.get_body ()));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_del_regex (gspc::net::frame const &rqst)
    {
      return
        encode_error_code (rqst, m_kvs->del_regex (rqst.get_body ()));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_set_ttl (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      if (pos.end ())
        return gspc::net::make::error_frame
          ( rqst
          , gspc::net::E_SERVICE_FAILED
          , "empty body"
          );

      int ttl = fhg::util::read<int>
        (fhg::util::parse::require::plain_string (pos, ' '));

      api_t::key_type key =
        fhg::util::parse::require::plain_string (pos, '\n');

      return encode_error_code (rqst, m_kvs->set_ttl (key, ttl));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_set_ttl_regex (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      if (pos.end ())
        return gspc::net::make::error_frame
          ( rqst
          , gspc::net::E_SERVICE_FAILED
          , "empty body"
          );

      const int ttl = fhg::util::read<int>
        (fhg::util::parse::require::plain_string (pos, ' '));

      const std::string regex =
        fhg::util::parse::require::string (pos);

      return encode_error_code (rqst, m_kvs->set_ttl_regex (regex, ttl));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_push (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      const api_t::key_type key =
        fhg::util::parse::require::plain_string (pos, ' ');
      const api_t::value_type val =
        pnet::type::value::read (pos);

      return encode_error_code (rqst, m_kvs->push (key, val));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_wait (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      if (pos.end ())
        return gspc::net::make::error_frame
          ( rqst
          , gspc::net::E_SERVICE_FAILED
          , "empty body"
          );

      int events = fhg::util::read<int>
        (fhg::util::parse::require::plain_string (pos, ' '));

      api_t::key_type key =
        fhg::util::parse::require::plain_string (pos, '\n');

      int rc = m_kvs->wait (key, events, 0);
      if (rc > 0)
      {
        return encode_error_code (rqst, rc);
      }
      else
      {
        waiting_t wobject;
        wobject.rqst = rqst;
        wobject.key = key;
        wobject.mask = events;
        wobject.events = 0;

        boost::unique_lock<boost::shared_mutex> lock (m_waiting_mtx);
        m_waiting.push_back (wobject);

        return boost::none;
      }
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_try_pop (gspc::net::frame const &rqst)
    {
      api_t::value_type val;
      int rc = m_kvs->try_pop (rqst.get_body (), val);
      gspc::net::frame rply = encode_error_code (rqst, rc);
      if (0 == rc)
      {
        gspc::net::stream (rply) << pnet::type::value::show (val) << std::endl;
      }
      return rply;
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_counter_reset (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      if (pos.end ())
        return gspc::net::make::error_frame
          ( rqst
          , gspc::net::E_SERVICE_FAILED
          , "empty body"
          );

      int val = fhg::util::read<int>
        (fhg::util::parse::require::plain_string (pos, ' '));

      api_t::key_type key =
        fhg::util::parse::require::plain_string (pos, '\n');

      return encode_error_code (rqst, m_kvs->counter_reset (key, val));
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_counter_change (gspc::net::frame const &rqst)
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      if (pos.end ())
        return gspc::net::make::error_frame
          ( rqst
          , gspc::net::E_SERVICE_FAILED
          , "empty body"
          );

      int delta = fhg::util::read<int>
        (fhg::util::parse::require::plain_string (pos, ' '));

      api_t::key_type key =
        fhg::util::parse::require::plain_string (pos, '\n');

      int val;
      int rc = m_kvs->counter_change (key, val, delta);

      gspc::net::frame rply = encode_error_code (rqst, rc);
      gspc::net::stream (rply) << val << std::endl;
      return rply;
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_help (gspc::net::frame const &rqst)
    {
      gspc::net::frame rply = gspc::net::make::reply_frame (rqst);
      gspc::net::stream (rply)
        << "available RPC:" << std::endl;
      rpc_table_t::const_iterator it = m_rpc_table.begin ();
      const rpc_table_t::const_iterator end = m_rpc_table.end ();

      for ( ; it != end ; ++it)
      {
        gspc::net::stream (rply) << "   " << it->first << std::endl;
      }

      return rply;
    }

    boost::optional<gspc::net::frame>
    service_t::rpc_invalid (gspc::net::frame const &rqst)
    {
      return gspc::net::make::error_frame
        ( rqst
        , gspc::net::E_SERVICE_NO_SUCH_RPC
        , "kvs"
        );
    }

    gspc::net::frame
    service_t::encode_error_code ( gspc::net::frame const &rqst
                                 , int rc
                                 )
    {
      gspc::net::frame rply = gspc::net::make::reply_frame (rqst);
      gspc::net::stream (rply) << rc << std::endl;
      return rply;
    }

    void service_t::operator () ( std::string const &dst
                                , gspc::net::frame const &rqst
                                , gspc::net::user_ptr user
                                )
    {
      rpc_table_t::const_iterator it = m_rpc_table.find (dst);
      if (it == m_rpc_table.end ())
      {
        gspc::net::frame rply = *rpc_invalid (rqst);
        user->deliver (rply);
      }
      else
      {
        boost::optional<gspc::net::frame> rply = it->second (rqst);
        if (rply)
          user->deliver (*rply);
      }
    }

    void service_t::on_change (api_t::key_type const &key, int events)
    {
      boost::unique_lock<boost::shared_mutex> lock (m_waiting_mtx);
      waiting_list_t::iterator it = m_waiting.begin ();
      const waiting_list_t::iterator end = m_waiting.end ();

      while (it != end)
      {
        if (it->key == key && it->mask & events)
        {
          gspc::net::frame rply = encode_error_code (it->rqst, events);
          gspc::net::server::default_queue_manager ().deliver (rply);

          it = m_waiting.erase (it);
        }
        else
        {
          ++it;
        }
      }
    }
  }
}
