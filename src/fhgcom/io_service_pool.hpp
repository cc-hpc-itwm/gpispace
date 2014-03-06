// basically adapted from
//    http://www.boost.org/doc/libs/1_44_0/doc/html/boost_asio/example/http/server2/io_service_pool.hpp

#ifndef FHGCOM_IO_SERVICE_POOL_HPP
#define FHGCOM_IO_SERVICE_POOL_HPP 1

#include <vector>

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
      explicit io_service_pool (std::size_t pool_size);

      void set_nthreads (size_t nthreads) { m_nthreads = nthreads; }

      void run ();
      void stop ();

      boost::asio::io_service & get_io_service ();
    private:
      typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;
      typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;

      std::vector<io_service_ptr> io_services_;
      std::vector<work_ptr> work_;

      std::size_t next_io_service_;
      std::size_t m_nthreads;
    };
  }
}

#endif
