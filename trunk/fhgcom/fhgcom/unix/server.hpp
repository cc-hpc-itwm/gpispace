#ifndef FHG_COM_UNIX_SERVER_HPP
#define FHG_COM_UNIX_SERVER_HPP 1

#include <string>

namespace fhg
{
  namespace com
  {
    namespace unix
    {
      class server
      {
      public:
        explicit
        server ( boost::asio::io_service & io_service
               , std::string const & path
               , int mode = 0600
               );

        void start (bool reuse_address = true);
      private:
        boost::asio::io_service & m_io_service;
        std::string m_path;
        int m_mode;
      };
    }
  }
}

#endif
