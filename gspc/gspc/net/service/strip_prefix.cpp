#include "strip_prefix.hpp"

#include <gspc/net/frame.hpp>

namespace gspc
{
  namespace net
  {
    namespace service
    {
      strip_prefix::strip_prefix ( std::string const &prefix
                                 , handler_t next
                                 )
        : m_prefix (prefix)
        , m_next (next)
      {}

      void strip_prefix::operator() ( std::string const &dst
                                    , frame const &rqst
                                    , user_ptr user
                                    )
      {
        const std::size_t pos = dst.find (m_prefix);
        if (0 == pos)
        {
          return m_next (dst.substr (m_prefix.size ()), rqst, user);
        }
        else
        {
          return m_next (dst, rqst, user);
        }
      }
    }
  }
}
