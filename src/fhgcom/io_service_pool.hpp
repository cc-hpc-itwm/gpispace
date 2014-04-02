// basically adapted from
//    http://www.boost.org/doc/libs/1_44_0/doc/html/boost_asio/example/http/server2/io_service_pool.hpp

#ifndef FHGCOM_IO_SERVICE_POOL_HPP
#define FHGCOM_IO_SERVICE_POOL_HPP 1

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace fhg
{
  namespace com
  {
    class io_service_pool
      : private boost::noncopyable
    {
    public:
      io_service_pool ();

      void set_nthreads (size_t nthreads) { m_nthreads = nthreads; }

      void run ();
      void stop ();

      boost::asio::io_service & get_io_service ();
    private:
      boost::shared_ptr<boost::asio::io_service> io_service_;
      boost::shared_ptr<boost::asio::io_service::work> work_;

      std::size_t m_nthreads;
    };
  }
}

#endif
