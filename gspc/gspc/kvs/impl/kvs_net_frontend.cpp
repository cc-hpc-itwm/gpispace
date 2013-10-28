#include "kvs_net_frontend.hpp"

#include <boost/format.hpp>

#include <gspc/net.hpp>

#include <fhg/util/read.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

namespace gspc
{
  namespace kvs
  {
    static std::string KVS_SERVICE = "/service/kvs";

    kvs_net_frontend_t::kvs_net_frontend_t (std::string const &url)
      : m_url (url)
      , m_client ()
    {
      boost::system::error_code ec;
      m_client = gspc::net::dial (m_url, ec);
      if (not m_client)
      {
        throw boost::system::system_error (ec);
      }
    }

    static void throw_if_error ( std::string const &service
                               , gspc::net::frame const &f
                               )
    {
      if (f.get_command () == "ERROR")
      {
        boost::system::error_code ec = gspc::net::make_error_code (f);
        if (ec)
        {
          throw std::runtime_error
            ((boost::format
             ( "'%1%' failed: %2% [%3%]: %4%")
             % service
             % ec.message ()
             % ec.value ()
             % f.get_body ()
             ).str ()
            );
        }
      }
    }

    int kvs_net_frontend_t::do_put (std::list<std::pair<key_type, value_type> > const &vals)
    {
      using namespace gspc::net;

      int rc;
      frame rply;
      frame rqst;

      rqst.set_header ("destination", KVS_SERVICE + "/put");

      typedef std::list<std::pair<key_type, value_type> > kv_list_t;
      kv_list_t::const_iterator it = vals.begin ();
      const kv_list_t::const_iterator end = vals.end ();

      for (; it != end ; ++it)
      {
        stream (rqst)
          << it->first
          << " "
          << pnet::type::value::show (it->second)
          << std::endl
          ;
      }

      rc = m_client->request (rqst, rply);
      if (rc != 0)
        return rc;

      throw_if_error ("put", rply);

      return 0;
    }

    int kvs_net_frontend_t::do_get (key_type const &key, value_type &val) const
    {
      int rc;

      using namespace gspc::net;

      frame rply;
      frame rqst;

      rqst.set_header ("destination", KVS_SERVICE + "/get");

      stream (rqst) << key << std::endl;

      rc = m_client->request (rqst, rply);
      if (rc != 0)
        return rc;

      throw_if_error ("get", rply);

      fhg::util::parse::position_string pos (rply.get_body ());

      const int return_code =
        fhg::util::read<int>
        (fhg::util::parse::require::plain_string (pos, ' '));

      if (0 == return_code)
      {
        val = pnet::type::value::read (pos);
      }

      return return_code;
    }

    int kvs_net_frontend_t::do_get_regex ( std::string const &regex
                                         , std::list<std::pair<key_type, value_type> > &
                                         ) const
    {
      return -ENOTSUP;
    }

    int kvs_net_frontend_t::do_del (key_type const &key)
    {
      return -ENOTSUP;
    }
    int kvs_net_frontend_t::do_del_regex (std::string const &regex)
    {
      return -ENOTSUP;
    }

    int kvs_net_frontend_t::do_set_ttl (key_type const &key, int ttl)
    {
      return -ENOTSUP;
    }
    int kvs_net_frontend_t::do_set_ttl_regex (std::string const &regex, int ttl)
    {
      return -ENOTSUP;
    }

    int kvs_net_frontend_t::do_push (key_type const &key, value_type const &val)
    {
      return -ENOTSUP;
    }
    int kvs_net_frontend_t::do_pop (key_type const &, value_type &val)
    {
      return -ENOTSUP;
    }
    int kvs_net_frontend_t::do_try_pop (key_type const &, value_type &val)
    {
      return -ENOTSUP;
    }

    int kvs_net_frontend_t::do_counter_reset (key_type const &key, int  val)
    {
      return -ENOTSUP;
    }

    int kvs_net_frontend_t::do_counter_change (key_type const &key, int &val, int delta)
    {
      return -ENOTSUP;
    }
  }
}
