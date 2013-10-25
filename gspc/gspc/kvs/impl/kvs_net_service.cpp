#include "kvs_net_service.hpp"

#include <iostream>
#include <string>

#include <gspc/net.hpp>
#include <gspc/kvs/kvs.hpp>

#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

#include <we/type/value/read.hpp>
#include <we/type/value/show.hpp>
#include <boost/optional.hpp>

namespace gspc
{
  namespace kvs
  {
    service_t::service_t ()
      : m_kvs (gspc::kvs::create ("inproc://"))
    {}

    service_t::service_t (std::string const &url)
      : m_kvs (gspc::kvs::create (url))
    {}

    void service_t::operator () ( std::string const &dst
                                , gspc::net::frame const &rqst
                                , gspc::net::user_ptr user
                                )
    {
      fhg::util::parse::position_string pos (rqst.get_body ());

      if      (dst == "put")
      {
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

        m_kvs->put (values_to_put);
        gspc::net::frame rply = gspc::net::make::reply_frame (rqst);
        user->deliver (rply);
      }
      else if (dst == "get")
      {
        const api_t::key_type key =
          fhg::util::parse::require::plain_string (pos, '\n');

        api_t::value_type val;
        int rc = m_kvs->get (key, val);

        gspc::net::frame rply = gspc::net::make::reply_frame (rqst);
        gspc::net::stream (rply)
          << rc
          << " "
          << pnet::type::value::show (val)
          << std::endl
          ;

        user->deliver (rply);
      }
      else if (dst == "push")
      {
      }
      else if (dst == "pop")
      {
      }
      else if (dst == "help")
      {
        gspc::net::frame rply = gspc::net::make::reply_frame (rqst);
        gspc::net::stream (rply)
          << "available RPC:" << std::endl
          << "    help" << std::endl
          << "    get"  << std::endl
          << "    put"  << std::endl
          << "    push" << std::endl
          << "    pop"  << std::endl
          << "    ttl"  << std::endl
          ;
        user->deliver (rply);
      }
      else
      {
        gspc::net::frame rply = gspc::net::make::error_frame
          (rqst, gspc::net::E_SERVICE_NO_SUCH_RPC, dst);
        user->deliver (rply);
      }

      /*
      protocol::Service service;


      fhg::util::parse::position_string pos (rqst.get_body ());

      if (pos.end ())
      {
        rply = gspc::net::make::error_frame
          (rqst, gspc::net::E_SERVICE_FAILED, "empty request");
      }
      else
      {
        try {
        while (not pos.end ())
        {
          switch (*pos)
          {
          case '\n':
            {
              ++pos;
            }
            break;
          case 'P':
            {
              ++pos;

              fhg::util::parse::require::require (pos, "UT");
              fhg::util::parse::require::require (pos, ' ');
              fhg::util::parse::require::skip_spaces (pos);
              const std::string key = fhg::util::parse::require::plain_string (pos, ' ');
              api_t::value_type val = pnet::type::value::read (pos);
              fhg::util::parse::require::skip_spaces (pos);

              int rc = m_kvs->put (key, val);

              if (not rply)
                rply = gspc::net::make::reply_frame (rqst);

              if (rc == 0)
              {
                gspc::net::stream (*rply)
                  << "OK" << " " << "PUT" << " " << key
                  << std::endl
                  ;
              }
              else
              {
                gspc::net::stream (*rply)
                  << "ERROR" << " " << "GET" << " " << key << " " << rc
                  << std::endl
                  ;
              }
            }
            break;
          case 'G':
            {
              ++pos;
              fhg::util::parse::require::require (pos, "ET");
              fhg::util::parse::require::require (pos, ' ');
              fhg::util::parse::require::skip_spaces (pos);
              const std::string key = fhg::util::parse::require::plain_string (pos, '\n');

              if (not rply)
                rply = gspc::net::make::reply_frame (rqst);

              api_t::value_type val;
              int rc = m_kvs->get (key, val);
              if (rc == 0)
              {
                gspc::net::stream (*rply)
                  << "VALUE" << " " << key << " " << pnet::type::value::show (val)
                  << std::endl
                  ;
              }
              else
              {
                gspc::net::stream (*rply)
                  << "ERROR" << " " << "GET" << " " << key << " " << rc
                  << std::endl
                  ;
              }
            }
            break;
          }
        }
        } catch (std::exception const &ex)
        {
          rply = gspc::net::make::error_frame
            (rqst, gspc::net::E_SERVICE_FAILED, std::string (ex.what ()));
        }
      }


      if (rply)
      {
        user->deliver (*rply);
      }
      else
      {
        // TODO: insert user into notification list...
      }
      */
    }

    void service_t::on_change (api_t::key_type const &key)
    {
      std::cerr << "key changed: " << key << std::endl;
    }
  }
}
