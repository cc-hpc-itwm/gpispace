#include "kvs_net_frontend.hpp"

#include <boost/thread/locks.hpp>
#include <boost/format.hpp>

#include <gspc/net/client.hpp>
#include <gspc/net/error.hpp>
#include <gspc/net/dial.hpp>
#include <gspc/net/frame.hpp>

#include <fhg/util/num.hpp>
#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>

#include <boost/system/system_error.hpp>

namespace gspc
{
  namespace kvs
  {
    static std::string KVS_SERVICE = "/service/kvs";

    kvs_net_frontend_t::kvs_net_frontend_t (std::string const &url)
      : m_client ()
    {
      boost::system::error_code ec;
      m_client = gspc::net::dial (url, ec);
      if (not m_client)
      {
        throw boost::system::system_error (ec);
      }
    }

    static int s_get_error (gspc::net::frame const &f)
    {
      if (f.get_command () == "ERROR")
      {
        boost::system::error_code ec = gspc::net::make_error_code (f);
        if (ec)
        {
          return -ec.value ();
        }
      }

      return 0;
    }

    int kvs_net_frontend_t::request ( std::string const &rpc
                                    , std::string const &rqst_body
                                    , std::string &rply_body
                                    , size_t timeout
                                    ) const
    {
      using namespace gspc::net;

      int rc;
      frame rply;
      frame rqst;

      {
        boost::lock_guard<boost::mutex> _ (m_mutex);

        if (not m_client->is_connected ())
        {
          m_client->stop ();
          rc = m_client->start ();
          if (rc != 0)
          {
            return rc;
          }
        }
      }

      rqst.set_header ("destination", KVS_SERVICE + "/" + rpc);
      rqst.set_body (rqst_body);

      if ((size_t)-1 == timeout)
      {
        rc = m_client->request (rqst, rply, boost::posix_time::pos_infin);
      }
      else if (0 < timeout)
      {
        rc = m_client->request (rqst, rply, boost::posix_time::milliseconds (timeout));
      }
      else
      {
        rc = m_client->request (rqst, rply);
      }

      if (rc != 0)
        return rc;

      rc = s_get_error (rply);
      if (rc != 0)
        return rc;

      rply_body = rply.get_body ();

      return rc;
    }

    namespace
    {
      int read_int_with_following_newline (std::string const& input)
      {
        fhg::util::parse::position_string pos (input);

        int const value (fhg::util::read_int (pos));
        fhg::util::parse::require::require (pos, '\n');

        return value;
      }
    }

    int kvs_net_frontend_t::do_put (std::list<std::pair<key_type, value_type> > const &vals)
    {
      typedef std::list<std::pair<key_type, value_type> > kv_list_t;
      kv_list_t::const_iterator it = vals.begin ();
      const kv_list_t::const_iterator end = vals.end ();

      std::ostringstream sstr;
      for (; it != end ; ++it)
      {
        sstr << it->first
             << " "
             << pnet::type::value::show (it->second)
             << std::endl
          ;
      }

      std::string rply;
      int rc = request ("put", sstr.str (), rply);
      if (rc != 0)
        return rc;

      return read_int_with_following_newline (rply);
    }

    int kvs_net_frontend_t::do_get (key_type const &key, value_type &val) const
    {
      std::string rply;
      int rc = request ("get", key, rply);

      if (rc != 0)
        return rc;

      fhg::util::parse::position_string pos (rply);

      rc = fhg::util::read_int (pos);
      fhg::util::parse::require::require (pos, '\n');

      if (0 == rc)
      {
        val = pnet::type::value::read (pos);
      }

      return rc;
    }

    int kvs_net_frontend_t::do_get_regex ( std::string const &regex
                                         , std::list<std::pair<key_type, value_type> > &values
                                         ) const
    {
      std::string rply;
      int rc;

      rc = request ("get_regex", regex, rply);

      if (rc != 0)
        return rc;

      fhg::util::parse::position_string pos (rply);
      rc = fhg::util::read_int (pos);
      fhg::util::parse::require::require (pos, '\n');

      if (rc != 0)
        return rc;

      while (not pos.end ())
      {
        if (*pos == '\n')
        {
          ++pos;
          continue;
        }

        const key_type key =
          fhg::util::parse::require::plain_string (pos, ' ');
        const value_type val =
          pnet::type::value::read (pos);
        values.push_back (std::make_pair (key, val));
      }

      return rc;
    }

    int kvs_net_frontend_t::do_del (key_type const &key)
    {
      std::string rply;
      int rc;

      rc = request ("del", key, rply);

      if (rc != 0)
        return rc;

      return read_int_with_following_newline (rply);
    }

    int kvs_net_frontend_t::do_del_regex (std::string const &regex)
    {
      std::string rply;
      int rc;

      rc = request ("del_regex", regex, rply);

      if (rc != 0)
        return rc;

      return read_int_with_following_newline (rply);
    }

    int kvs_net_frontend_t::do_set_ttl (key_type const &key, int ttl)
    {
      std::string rply;
      int rc;

      std::ostringstream sstr;
      sstr << ttl << " " << key << std::endl;

      rc = request ("set_ttl", sstr.str (), rply);

      if (rc != 0)
        return rc;

      return read_int_with_following_newline (rply);
    }

    int kvs_net_frontend_t::do_set_ttl_regex (std::string const &regex, int ttl)
    {
      std::string rply;
      int rc;

      std::ostringstream sstr;
      sstr << ttl << ' ' << '"' << regex << '"' << std::endl;

      rc = request ("set_ttl_regex", sstr.str (), rply);

      if (rc != 0)
        return rc;

      return read_int_with_following_newline (rply);
    }

    int kvs_net_frontend_t::do_push (key_type const &key, value_type const &val)
    {
      std::ostringstream sstr;
      sstr << key << " " << pnet::type::value::show (val) << std::endl;

      std::string rply;
      int rc = request ("push", sstr.str (), rply);
      if (rc != 0)
        return rc;

      return read_int_with_following_newline (rply);
    }

    int kvs_net_frontend_t::do_try_pop (key_type const &key, value_type &val)
    {
      std::string rply;
      int rc = request ("try_pop", key, rply);

      if (rc != 0)
        return rc;

      fhg::util::parse::position_string pos (rply);

      rc = fhg::util::read_int (pos);
      fhg::util::parse::require::require (pos, '\n');

      if (0 == rc)
      {
        val = pnet::type::value::read (pos);
      }

      return rc;
    }

    int kvs_net_frontend_t::do_counter_reset (key_type const &key, int  val)
    {
      std::ostringstream sstr;
      sstr << val << " " << key << std::endl;

      std::string rply;
      int rc = request ("counter_reset", sstr.str (), rply);

      if (rc != 0)
        return rc;

      return read_int_with_following_newline (rply);
    }

    int kvs_net_frontend_t::do_counter_change (key_type const &key, int &val, int delta)
    {
      std::ostringstream sstr;
      sstr << delta << " " << key << std::endl;

      std::string rply;
      int rc = request ("counter_change", sstr.str (), rply);

      if (rc != 0)
        return rc;

      std::istringstream isstr (rply);
      isstr >> rc;
      if (rc != 0)
        return rc;

      isstr >> val;
      if (isstr.bad ())
      {
        return -EBADMSG;
      }

      return rc;
    }

    int kvs_net_frontend_t::do_wait ( key_type const &key
                                    , int mask
                                    , int timeout
                                    ) const
    {
      int rc;
      std::ostringstream rqst;
      rqst << mask << " " << key << std::endl;

      std::string rply;
      rc = request ("wait", rqst.str (), rply, timeout);

      if (rc != 0)
        return rc;

      return read_int_with_following_newline (rply);
    }
  }
}
