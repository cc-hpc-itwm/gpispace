#include "strip_prefix.hpp"

#include <gspc/net/frame.hpp>

namespace gspc
{
  namespace net
  {
    namespace handler
    {
      strip_prefix::strip_prefix ( std::string const &prefix
                                 , gspc::net::server::service_handler_t next
                                 )
        : m_prefix (prefix)
        , m_next (next)
      {}

      void strip_prefix::operator() ( std::string const &dst
                                    , frame const &rqst
                                    , frame & rply
                                    )
      {
        std::string new_dst (dst);

        std::size_t found = dst.find (m_prefix);
        if (found == 0)
        {
          new_dst.replace (0, m_prefix.size (), "");
        }

        m_next (new_dst, rqst, rply);
      }
    }
  }
}
