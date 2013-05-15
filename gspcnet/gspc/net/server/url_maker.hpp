#ifndef GSPC_NET_SERVER_URL_MAKER_HPP
#define GSPC_NET_SERVER_URL_MAKER_HPP

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <class Proto>
      struct url_maker
      {
        typedef typename Proto::endpoint endpoint_type;

        static std::string make (endpoint_type const &);
      };
    }
  }
}

#endif
