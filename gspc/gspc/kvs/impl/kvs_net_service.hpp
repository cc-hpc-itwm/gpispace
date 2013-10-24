#ifndef GSPC_KVS_SERVICE_HPP
#define GSPC_KVS_SERVICE_HPP

#include <boost/shared_ptr.hpp>

#include <gspc/kvs/api.hpp>
#include <gspc/net/service/handler.hpp>

namespace gspc
{
  namespace kvs
  {
    class service_t
    {
    public:
      static const std::string &NAME ()
      {
        static std::string name ("kvs"); return name;
      }

      service_t ();

      explicit
      service_t (std::string const &url);

      void operator () ( std::string const &dst
                       , gspc::net::frame const &rqst
                       , gspc::net::user_ptr user
                       );
    private:
      void on_change (api_t::key_type const &);

      boost::shared_ptr<api_t> m_kvs;
    };
  }
}

#endif
